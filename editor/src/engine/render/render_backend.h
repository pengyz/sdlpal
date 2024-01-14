#pragma once
#include "3rd/SDL/include/SDL_surface.h"
#include <SDL.h>

namespace render {

class RenderBackend {
public:
    RenderBackend(SDL_Renderer* renderer, SDL_Surface* surface, bool bKeepAspectRatio);
    virtual ~RenderBackend();

    // 绘制函数
    virtual void Init() = 0;
    virtual void Setup() = 0;
    virtual SDL_Texture* CreateTexture(int width, int height) = 0;
    virtual void RenderCopy() = 0;

    // getter and setter
    SDL_Renderer* renderer() const;
    bool keepAspectRatio() const;
    void keepAspectRatio(bool val);
    SDL_Texture* texture() const;

protected:
    SDL_Renderer* _renderer = nullptr; // renderer对象
    SDL_Rect _textureRect; // rect对象
    SDL_Surface* _screenReal = nullptr; // gpScreenReal
    SDL_Texture* _texture = nullptr; // 游戏纹理对象 GPU纹理
    bool _bKeepAspectRatio = true; // 是否保持宽高比
    int _textureWidth = 640; // texture宽度
    int _textureHeight = 400; // texture高度
};
} // namespace render
