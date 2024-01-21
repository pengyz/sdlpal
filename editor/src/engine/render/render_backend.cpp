#include "render_backend.h"
#include "SDL.h"

namespace render {
RenderBackend::RenderBackend(SDL_Renderer* renderer, SDL_Surface* surface, bool bKeepAspectRatio)
    : _renderer(renderer)
    , _bKeepAspectRatio(bKeepAspectRatio)
    , _screenReal(surface)
{
}
RenderBackend::~RenderBackend()
{
}

SDL_Renderer* RenderBackend::renderer() const { return _renderer; }
bool RenderBackend::keepAspectRatio() const { return _bKeepAspectRatio; }
void RenderBackend::keepAspectRatio(bool val) { _bKeepAspectRatio = val; }
SDL_Texture* RenderBackend::texture() const { return _texture; }

} // namespace render