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

float SineSurface(const int x, const int y, const float a, const float b, const float c) {
    return Sine(x+y, a, b, c, 0.f);
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

std::tuple<float, float, float> Torus(const float t, const float s, const float R, const float r, const float x0, const float y0) {

    float x = x0 + (R + r * glm::cos(t)) * glm::cos(s);
    float y = y0 + (R + r * glm::cos(t)) * glm::sin(s);
    float z = r * glm::sin(t);

    return {x, y, z};
}

std::tuple<float, float, float> Sphere(const float t, const float s, const float r, const float x0, const float y0) {

    float x = x0 + r * glm::sin(t) * glm::cos(s);
    float y = y0 + r * glm::sin(t) * glm::sin(s);
    float z = r * glm::cos(t);

    return {x, y, z};
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
        info.function = std::bind(SineSurface, std::placeholders::_1, std::placeholders::_2, 5.f, 0.1f, 0.f);
        info.colorlo = 0xffff7f00;
        info.colorhi = 0xfffe900;

        grapher.AddSurface(info);
    }

    {
        GR::Grapher::FunctionInfo info;

        info.color = 0xfffd0000;

        for (int i = 0; i < 25; ++i) {
            info.function = std::bind(Linear, std::placeholders::_1, 0.f, static_cast<float>(surface->h) - static_cast<float>(i) * (static_cast<float>(i) * 0.07f) * 3.f);
            grapher.AddFunction(info);
        }
    }

    {
        GR::Grapher::FunctionInfo info;

        info.color = 0xffffd644;
        info.axis = GR::Axis::Y;
        info.plot = GR::PlotType::LINE;

        for (int i = 0; i < 7; ++i) {
            info.function = std::bind(Sine, std::placeholders::_1, 10.f, 0.1f, static_cast<float>(i) * 1.f, 15.f + static_cast<float>(i) * 30.f);

            grapher.AddFunction(info);
        }
    }

    {
        GR::Grapher::EquationInfo info;
        info.plot = GR::PlotType::LINE;
        info.color = 0xff23d6ff;

        for (int i = 0; i < 10; ++i) {
            info.equation = std::bind(Circle, std::placeholders::_1, static_cast<float>(surface->w) * 0.2f + 100.f * static_cast<float>(i), static_cast<float>(surface->h) * 0.75 - 15.f * static_cast<float>(i), 40.f);
            info.t0 = 0.f;
            info.tMax = TWOPI + 0.5f * PI;
            info.tStep = TWOPI / (3.f + static_cast<float>(i));

            grapher.AddEquation(info);
        }

        info.color = 0xff88ff84;

        info.t0 = 0.f;
        info.tMax = TWOPI + 0.5f * PI;
        info.tStep = TWOPI / 3.f;
        for (int i = 0; i < 9; ++i) {
            float angle = glm::radians(static_cast<float>(i) * 40.f);
            info.equation = std::bind(Circle, std::placeholders::_1,
                SCR_WIDTH * 0.7f + static_cast<float>(i) * glm::cos(angle) * 35.f,
                SCR_HEIGHT * 0.4f + static_cast<float>(i) * glm::sin(angle) * 35.f,
                static_cast<float>(i + 1) * 10.f);
            grapher.AddEquation(info);
        }
    }

    {
        GR::Grapher::ParametricSurfaceInfo info;


        info.colorlo = 0xff000000;
        info.colorhi = 0xffffffff;

        info.t0 = 0.f;
        info.tMax = TWOPI + 0.5f * PI;
        info.tStep = TWOPI / 360.f;
        info.s0 = 0.f;
        info.sMax = TWOPI + 0.5f * PI;
        info.sStep = TWOPI / 360.f;

        for (int i = 0; i < 3; ++i) {
            info.function = std::bind(Sphere, std::placeholders::_1, std::placeholders::_2, 120.f - static_cast<float>(i) * 20.f, SCR_WIDTH * 0.15f + 150.f * static_cast<float>(i), SCR_HEIGHT * 0.15f+ 30.f * static_cast<float>(i));

            grapher.AddParametricSurface(info);
        }
    }

    {
        GR::Grapher::ParametricSurfaceInfo info;

        info.function = std::bind(Torus, std::placeholders::_1, std::placeholders::_2, 100.f, 20.f, SCR_WIDTH * 0.75f, SCR_HEIGHT * 0.75f);
        info.t0 = 0.f;
        info.tMax = TWOPI + 0.5f * PI;
        info.tStep = TWOPI / 360.f;
        info.s0 = 0.f;
        info.sMax = TWOPI + 0.5f * PI;
        info.sStep = TWOPI / 720.f;

        grapher.AddParametricSurface(info);
    }

    /*
    {
        GR::Grapher::SurfaceInfo info;
        info.function = std::bind(SlopeSurface, std::placeholders::_1, std::placeholders::_2, 5.f, 0.1f, 0.f);
        info.colorlo = 0xff0f00ff;
        info.colorhi = 0xfff00f00;
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

    { // Parametric surface
        GR::Grapher::ParametricSurfaceInfo info;

        info.function = std::bind(Torus, std::placeholders::_1, std::placeholders::_2, 100.f, 20.f, 200.f, 200.f);
        info.t0 = 0.f;
        info.tMax = TWOPI + 0.5f * PI;
        info.tStep = TWOPI / 360.f;
        info.s0 = 0.f;
        info.sMax = TWOPI + 0.5f * PI;
        info.sStep = TWOPI / 720.f;

        grapher.AddParametricSurface(info);
    }
    */

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
                    if (event.key.key == SDLK_ESCAPE) {
                        running = false;
                    }
                    if (event.key.key == SDLK_P) {

                        std::cout << "Saving frame to BMP" << std::endl;
                        SDL_SaveBMP(surface, "out.bmp");
                        // framebuffer to png
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
