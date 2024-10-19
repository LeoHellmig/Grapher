#pragma once
#include <functional>

// TODO
//
//  Color gradient for surfaces
//  Add height output to parametric?
//  Polygons
//  Parametric surface -> Parametric functions, two parameters 3D output
//


namespace GR
{
    enum class PlotType
    {
        PIXEL,
        LINE,
        SPLINE
    };

    class Grapher
    {
    public:
        struct FunctionInfo {
            std::function<float(int)> function;
            PlotType plot = PlotType::PIXEL;
            uint32_t color = 0xffffffff;
        };

        struct SurfaceInfo {
            std::function<float(int, int)> function;
            uint32_t color = 0xffffffff;

            // Normalized color
            // TODO Color gradient
            // TODO threshhold?
        };

        struct EquationInfo {
            std::function<std::pair<float, float>(float)> equation;
            PlotType plot = PlotType::PIXEL;
            uint32_t color = 0xffffffff;
            float t0;
            float tMax;
            float tStep;
        };

    private:

        enum class FuncType {
            FUNCTION,
            SURFACE,
            EQUATION
        };

        std::vector<std::pair<FuncType, size_t>> _order;

        std::vector<FunctionInfo> _functions;
        std::vector<SurfaceInfo> _surfaces;
        std::vector<EquationInfo> _equations;

        struct SurfaceWrapper {
            uint32_t* pixels;
            int width;
            int height;
        };

        struct Point {
            float x1, y1;
        };

        union Pixel {
            uint32_t uint;
            char8_t bytes[4];
        };

    public:
        void AddFunction(const FunctionInfo& functionInfo) {
            _functions.emplace_back(functionInfo);
            _order.emplace_back(FuncType::FUNCTION, _functions.size() - 1);
        }

        void AddSurface(const SurfaceInfo& surfaceInfo) {
            _surfaces.emplace_back(surfaceInfo);
            _order.emplace_back(FuncType::SURFACE, _surfaces.size() - 1);
        }

        void AddEquation(const EquationInfo& equationInfo) {
            _equations.emplace_back(equationInfo);
            _order.emplace_back(FuncType::EQUATION, _equations.size() - 1);
        }

        static void Plot(const int x,const int y,const uint32_t color, const SurfaceWrapper& surface)
        {
            if (x < 0 || x >= surface.width || y < 0 || y >= surface.height) {
                return;
            }
            uint32_t* pixel = surface.pixels + (x + y * surface.width);
            *pixel = color;
        }

#define OUTCODE(x,y) ((((x)<xmin)?1:(((x)>xmax)?2:0))+(((y)<ymin)?4:(((y)>ymax)?8:0)))
        static void Line(float x1, float y1, float x2, float y2, const uint32_t color, const SurfaceWrapper& surface)
        {
            // clip (Cohen-Sutherland, https://en.wikipedia.org/wiki/Cohen%E2%80%93Sutherland_algorithm)
            const float xmin = 0, ymin = 0, xmax = static_cast<float>(surface.width) - 1, ymax = static_cast<float>(surface.height) - 1;
            int c0 = OUTCODE( x1, y1 ), c1 = OUTCODE( x2, y2 );
            bool accept = false;
            while (true)
            {
                if (!(c0 | c1)) { accept = true; break; }
                if (c0 & c1) break;
                {
                    float x = 0, y = 0;
                    const int co = c0 ? c0 : c1;
                    if (co & 8) x = x1 + (x2 - x1) * (ymax - y1) / (y2 - y1), y = ymax;
                    else if (co & 4) x = x1 + (x2 - x1) * (ymin - y1) / (y2 - y1), y = ymin;
                    else if (co & 2) y = y1 + (y2 - y1) * (xmax - x1) / (x2 - x1), x = xmax;
                    else if (co & 1) y = y1 + (y2 - y1) * (xmin - x1) / (x2 - x1), x = xmin;
                    if (co == c0) x1 = x, y1 = y, c0 = OUTCODE( x1, y1 );
                    else x2 = x, y2 = y, c1 = OUTCODE( x2, y2 );
                }
            }
            if (!accept) return;
            float b = x2 - x1;
            float h = y2 - y1;
            float l = fabsf( b );
            if (fabsf( h ) > l) l = fabsf( h );
            const int il = static_cast<int>(l);
            const float dx = b / l;
            const float dy = h / l;
            for (int i = 0; i <= il; i++)
            {
                *(surface.pixels + static_cast<int>(x1) + static_cast<int>(y1) * surface.width) = color;
                x1 += dx, y1 += dy;
            }
        }

        static void DrawFunction(const SurfaceWrapper& surface, const FunctionInfo& functionInfo)
        {
            switch (functionInfo.plot) {
                case PlotType::PIXEL:
                    for (int x = 0; x < surface.width; ++x)
                    {
                        Plot(x, static_cast<int>(functionInfo.function(x)), functionInfo.color, surface);
                    }
                    break;
                case PlotType::LINE:
                    // Collect points
                    std::vector<Point> points;
                    points.reserve(surface.width);
                    for (int x = 0; x < surface.width; ++x) {
                        points.emplace_back(static_cast<float>(x), functionInfo.function(x));
                    }

                    for (int i = 0; i < points.size() - 1; ++i) {
                        Point a = points[i];
                        Point b = points[i + 1];
                        Line(a.x1, a.y1, b.x1, b.y1, functionInfo.color, surface);
                    }
                    break;
            }
        }

        static void DrawSurface(const SurfaceWrapper& surface, const SurfaceInfo& surfaceInfo) {
            std::vector<float> values;
            values.reserve(surface.width * surface.height);

            float min = INFINITY;
            float max = -INFINITY;

            for (int y = 0; y < surface.height; ++y) {
                for (int x = 0; x < surface.width; ++x) {

                    float res = surfaceInfo.function(x, y);

                    min = std::min(min, res);
                    max = std::max(max, res);

                    values.emplace_back(res);
                }
            }

            for (int y = 0; y < surface.height; ++y) {
                for (int x = 0; x < surface.width; ++x) {
                    // Todo cool stuff here
                    Pixel pixel{};
                    const uint32_t normalized = std::min(255, static_cast<int>(values[x + y*surface.width] / (max - min) * 255.f));
                    pixel.bytes[3] = 0xff;
                    pixel.bytes[2] = normalized;
                    pixel.bytes[1] = normalized;
                    pixel.bytes[0] = normalized;
                    Plot(x, y, pixel.uint, surface);
                }
            }
        }

        static void DrawEquation(const SurfaceWrapper& surface, const EquationInfo& equationInfo) {
            if (equationInfo.tStep == 0.f || (equationInfo.t0 > equationInfo.tMax && equationInfo.tStep < 0.f)) {
                return;
            }

            std::vector<Point> points;
            points.reserve(static_cast<size_t>((equationInfo.tMax - equationInfo.t0) / equationInfo.tStep));

            float t = equationInfo.t0;
            while (t < equationInfo.tMax) {

                auto res = equationInfo.equation(t);
                points.emplace_back(res.first, res.second);

                t += equationInfo.tStep;
            }

            switch (equationInfo.plot) {
                case PlotType::PIXEL:
                    for (Point point : points) {
                        Plot(static_cast<int>(point.x1), static_cast<int>(point.y1), equationInfo.color, surface);
                    }
                    break;
                case PlotType::LINE:
                    for (size_t i = 0; i < points.size() - 1; ++i) {
                        Point a = points[i];
                        Point b = points[i+1];
                        Line(a.x1, a.y1, b.x1, b.y1, equationInfo.color, surface);
                    }
                    break;
            }

        }

        void DrawAll(uint32_t* pixels, const int width, const int height)
        {
            const SurfaceWrapper surface = {pixels, width, height};

            for (auto pair : _order) {
                switch (pair.first) {
                    case FuncType::FUNCTION:
                        DrawFunction(surface, _functions[pair.second]);
                        break;
                    case FuncType::SURFACE:
                        DrawSurface(surface, _surfaces[pair.second]);
                        break;
                    case FuncType::EQUATION:
                        DrawEquation(surface, _equations[pair.second]);
                        break;
                    default: ;
                }
            }
        }
    };
}