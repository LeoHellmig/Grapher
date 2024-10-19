#include <iostream>
#include <SDL3/SDL.h>
#include <glm/glm.hpp>
#include "grapher.hpp"

constexpr int SCR_WIDTH = 1280;
constexpr int SCR_HEIGHT = 720;

constexpr float PI = 3.14159265359f;
constexpr float TWOPI = PI * 2.f;

SDL_Window* window;
SDL_Surface* surface;
uint32_t* pixels;

float Linear(const int x, const float c, const float o) {
    return static_cast<float>(x) * c + o;
}

float Sine(const int x, const float a, const float b, const float c, const float d) {
    return a * glm::sin(b * static_cast<float>(x) + c) + d;
}

float SlopeSurface(const int x, const int y, const float a, const float b, const float c) {
    return Sine(x, a, b, c, 0.f)+static_cast<float>(y);
}

std::pair<float, float> Parametric(const float t, const float o) {

    float x = glm::sin(0.1f * t) * 10.f + t * 1.3f + o;
    float y = t;

    return {x, y};
}

std::pair<float, float> Circle(const float t, const float x0, const float y0, const float r) {

    float x = x0 + glm::sin(t) * r;
    float y = y0 + glm::cos(t) * r;

    return {x, y};
}

int main()
{
    std::cout << "Hello, world!" << std::endl;

    if (SDL_Init(SDL_INIT_VIDEO) == 0)
    {
        SDL_Log("SDL failed to initialize");
        return -2;
    }

    unsigned int flags = 0;

    window = SDL_CreateWindow("Grapher", SCR_WIDTH, SCR_HEIGHT, flags);

    surface = SDL_GetWindowSurface(window);
    pixels = static_cast<uint32_t*>(surface->pixels);

    GR::Grapher grapher;

    {
        GR::Grapher::SurfaceInfo info;
        info.function = std::bind(SlopeSurface, std::placeholders::_1, std::placeholders::_2, 30.f, 0.1f, 0.f);
        grapher.AddSurface(info);
    }

    {
        GR::Grapher::FunctionInfo funcInfo;

        funcInfo.plot = GR::PlotType::LINE;
        funcInfo.color = 0xff0f0f0f;

        for (int i = 0; i < 30 ; i++) {
            funcInfo.function = std::bind(Linear, std::placeholders::_1, 0.02f * static_cast<float>(i), 0.f);
            grapher.AddFunction(funcInfo);
        }
    }


    {
        GR::Grapher::FunctionInfo funcInfo;
        funcInfo.function = std::bind(Sine, std::placeholders::_1, 90.f, 0.05f, 0.f, 360.f);
        funcInfo.plot = GR::PlotType::LINE;
        funcInfo.color = 0xffffff00;

        grapher.AddFunction(funcInfo);

        funcInfo.function = std::bind(Sine, std::placeholders::_1, 90.f, 0.05f, 0.f, 480.f);
        funcInfo.plot = GR::PlotType::PIXEL;
        funcInfo.color = 0xffffff00;

        grapher.AddFunction(funcInfo);
    }

    {
        GR::Grapher::EquationInfo info;
        info.equation = std::bind(Parametric, std::placeholders::_1, 0.f);
        info.plot = GR::PlotType::PIXEL;
        info.color = 0xff00ff00;
        info.t0 = 0.f;
        info.tMax = surface->h;
        info.tStep = 1.0f;

        grapher.AddEquation(info);
    }

    {// circles
        GR::Grapher::EquationInfo info;
        info.plot = GR::PlotType::LINE;
        info.color = 0xffff0f00;

        for (int i = 0; i < 10;  ++i) {
            info.equation = std::bind(Circle, std::placeholders::_1, 75.f + 125.f * static_cast<float>(i), SCR_HEIGHT - 75.f, 50.f);
            info.t0 = 0.f;
            info.tMax = TWOPI + 0.5f * PI;
            info.tStep = TWOPI / (3.f + static_cast<float>(i));

            grapher.AddEquation(info);
        }
    }

    bool running = true;
    bool draw = true;

    while (running)
    {
        SDL_Event event;
        if (SDL_PollEvent(&event) > 0)
        {
            switch (event.type)
            {
                case SDL_EVENT_KEY_DOWN:
                    if (event.key.key == SDLK_D) {
                        draw = true;
                    }
                break;
                case SDL_EVENT_QUIT:
                    running = false;
                break;
            }
        }
        SDL_LockSurface(surface);

        if (draw) {
            // Clear buffer
            memset(pixels, 40, SCR_WIDTH * SCR_HEIGHT * 4);

            // Draw in order
            grapher.DrawAll(static_cast<uint32_t *>(surface->pixels),surface->w, surface->h);
            draw = false;
        }

        SDL_UnlockSurface(surface);
        SDL_UpdateWindowSurface(window);
    }

    SDL_DestroyWindow(window);
    SDL_Quit();

    return 0;
}
