#include "mcv_platform.h"
#include "video_texture.h"
#include "utils/data_provider.h"
#include "background_player.h"

#pragma comment(lib, "h264bsd")

CVideoTexture::~CVideoTexture() {

    close();
}

bool CVideoTexture::initDecoder() {

    decoder = h264bsdAlloc();
    status = h264bsdInit(decoder, 0);
    if (status > 0) return false;

    len = (u32)data.size();
    byteStrm = data.data();
    num_frame = 0;
    picData = nullptr;
    finished = false;

    return true;
}

bool CVideoTexture::create(const std::string& name) {

    // Read the whole file...
    CFileDataProvider dp(name.c_str());
    if (!dp.isValid())
        return false;

    data.resize(dp.fileSize());
    dp.readBytes(data.data(), data.size());

    if (!initDecoder())
        return false;

    decodeNextFrame();

    // Creates the texture obj
    int new_xres = h264bsdPicWidth(decoder) * 16;
    int new_yres = h264bsdPicHeight(decoder) * 16;
    if (new_xres != xres || new_yres != yres) {
        if (!CTexture::create(
            new_xres, new_yres
            , DXGI_FORMAT_B8G8R8A8_UNORM
            , CTexture::CREATE_DYNAMIC
        ))
            return false;
    }

    uploadToVRAM();

    addVideoTextureToBackgroundPlayer(this);

    return true;
}

bool CVideoTexture::uploadToVRAM() {

    if (!picData)
        return false;

    //PROFILE_FUNCTION("CVideoTexture::VRAM");

    D3D11_MAPPED_SUBRESOURCE sub_resource;
    HRESULT hr = Render.ctx->Map(texture, 0, D3D11_MAP_WRITE_DISCARD, 0, &sub_resource);
    if (FAILED(hr))
        return false;
    int bytes_per_row = xres * 4;
    const u8* src = (u8*)picData;
    u8* dst = (u8*)sub_resource.pData;

    // Copy each line, in the gpu data might have some padding
    for (int i = 0; i < yres; ++i) {
        memcpy(dst, src, bytes_per_row);
        dst += sub_resource.RowPitch;
        src += bytes_per_row;
    }
    Render.ctx->Unmap(texture, 0);
    frame_ready = false;
    return true;
}

// return true if the video has finished
bool CVideoTexture::decodeNextFrame() {

    u32 bytesRead = 0;

    while (len > 0) {
        status = h264bsdDecode(decoder, byteStrm, len, 0, &bytesRead);
        byteStrm += bytesRead;
        len -= bytesRead;

        if (status == H264BSD_PIC_RDY) {
            ++num_frame;
            u32 picId, isIdrPic, numErrMbs;
            //u8* picData = h264bsdNextOutputPicture(decoder, &picId, &isIdrPic, &numErrMbs);
            picData = h264bsdNextOutputPictureBGRA(decoder, &picId, &isIdrPic, &numErrMbs);
            frame_ready = true;
            return false;
        }
        else if (status == H264BSD_HDRS_RDY) {
            auto vui = decoder->activeSps->vuiParameters;
            dbg("VideoDecoder: Header ready: TimeScale:%d fps:%1.2f\n", vui->timeScale, vui->timeScale * 0.5f);
            chrono.reset();
        }

        else if (status == H264BSD_ERROR) {
            dbg("VideoDecoder: General Error with %i bytes left\n", len + bytesRead);
        }

        else if (status == H264BSD_PARAM_SET_ERROR) {
            dbg("VideoDecoder: Param Set Error with %i bytes left\n", len + bytesRead);
        }

        else if (status == H264BSD_MEMALLOC_ERROR) {
            dbg("VideoDecoder: Malloc Error with %i bytes left\n", len + bytesRead);
        }

        status = H264BSD_RDY;
    }

    finished = true;
    return true;
}

void CVideoTexture::close() {

    if (decoder) {
        h264bsdShutdown(decoder);
        h264bsdFree(decoder);
        decoder = nullptr;
    }
}

bool CVideoTexture::isFrameReadyToUpload() const {

    return frame_ready;
}

// Called from a background thread...
void CVideoTexture::update(float dt) {

    decodeNextFrame();

    // Rewind those that finished.
    if (hasFinished()) {

        float time_to_play = chrono.elapsedAndReset();

        close();
        initDecoder();

        // Reload
        CFileDataProvider dp(name.c_str());
        if (dp.isValid())
            dp.readBytes(data.data(), data.size());
    }

}
