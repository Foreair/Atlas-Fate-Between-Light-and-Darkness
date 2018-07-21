#include "mcv_platform.h"
#include "comp_chrom_aberration.h"
#include "render/texture/render_to_texture.h"
#include "resources/resources_manager.h"
#include "render/render_objects.h"

DECL_OBJ_MANAGER("chromatic_aberration", TCompChromaticAberration);

// ---------------------
void TCompChromaticAberration::debugInMenu() {

    ImGui::Checkbox("Enabled", &enabled);
    ImGui::DragFloat("Amount", &cb_postfx.postfx_ca_amount, 0.01f, 0.0f, 1.0f);
    ImGui::DragFloat("Offset", &cb_postfx.postfx_ca_offset, 0.01f, 0.0f, 1.0f);
    ImGui::DragFloat("Shifting", &cb_postfx.postfx_cs_offset, 0.01f, 0.0f, 1.0f);
}

void TCompChromaticAberration::load(const json& j, TEntityParseContext& ctx) {

    enabled = j.value("enabled", true);
    amount = j.value("amount", 1.0f);
    int xres = Render.width;
    int yres = Render.height;

    if (!rt) {
        rt = new CRenderToTexture;
        // Create a unique name for the rt 
        char rt_name[64];
        sprintf(rt_name, "Chromatic_Aberration_%08x", CHandle(this).asUnsigned());
        bool is_ok = rt->createRT(rt_name, xres, yres, DXGI_FORMAT_B8G8R8A8_UNORM);
        assert(is_ok);
    }

    tech = Resources.get("postfx_chromatic_aberration.tech")->as<CRenderTechnique>();
    mesh = Resources.get("unit_quad_xy.mesh")->as<CRenderMesh>();

    cb_postfx.postfx_ca_amount = 0.09f;
    cb_postfx.postfx_ca_offset = 0.09f;
    cb_postfx.postfx_cs_offset = 0.5f;

}

CTexture* TCompChromaticAberration::apply(CTexture* in_texture) {

    cb_postfx.updateGPU();

    if (!enabled)
        return in_texture;

    CTraceScoped scope("TCompChromaticAberration");
    cb_globals.updateGPU();

    rt->activateRT();
    in_texture->activate(TS_ALBEDO);
    tech->activate();
    mesh->activateAndRender();

    return rt;
}