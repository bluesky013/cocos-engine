import {ccclass} from "../../core/data/decorators";
import {TextureBase} from "./texture-base";
import {PixelFormat} from "./asset-enum";
import {legacyCC} from "../../core/global-exports";

/**
 * @en The create information for [[VideoTexture]]
 * @zh 创建视频贴图的信息。
 */
export interface IVideoTextureCreateInfo {
    /**
     * @en The pixel width
     * @zh 像素宽度。
     */
    width: number;

    /**
     * @en The pixel height
     * @zh 像素高度。
     */
    height: number;
}

@ccclass('cc.VideoTexture')
export class VideoTexture extends TextureBase {
    public initialize (info: IVideoTextureCreateInfo) {
        this._width = info.width;
        this._height = info.height;
        this._setGFXFormat(PixelFormat.RGBA8888);
    }
}

legacyCC.VideoTexture = VideoTexture;
