#pragma once
#include <functional>
#include <map>

namespace GR
{
    enum class PlotType
    {
        PIXEL,
        LINE
    };

    enum class Axis {
        X,
        Y
    };

    class Grapher
    {
    public:
        union Pixel {
            Pixel(const uint32_t c) { uint = c; }
            uint32_t uint;
            char8_t bytes[4];
        };

        struct FunctionInfo {
            std::function<float(int)> function;
            PlotType plot = PlotType::PIXEL;
            Axis axis = Axis::X;
            Pixel color = 0xffffffff;
        };

        struct SurfaceInfo {
            std::function<float(int, int)> function;
            Pixel colorlo = 0xff000000;
            Pixel colorhi = 0xffffffff;
        };

        struct EquationInfo {
            std::function<std::pair<float, float>(float)> equation;
            PlotType plot = PlotType::PIXEL;
            Pixel color = 0xffffffff;
            float t0;
            float tMax;
            float tStep;
        };

        struct ParametricSurfaceInfo {
            std::function<std::tuple<float, float, float>(float, float)> function;
            Pixel colorlo = 0xff000000;
            Pixel colorhi = 0xffffffff;
            float t0;
            float tMax;
            float tStep;
            float s0;
            float sMax;
            float sStep;
        };

    private:

        enum class FuncType {
            FUNCTION,
            SURFACE,
            EQUATION,
            PARAMSURFACE
        };

        std::vector<std::pair<FuncType, size_t>> _order;

        std::vector<FunctionInfo> _functions;
        std::vector<SurfaceInfo> _surfaces;
        std::vector<EquationInfo> _equations;
        std::vector<ParametricSurfaceInfo> _parametricSurfaces;

        struct SurfaceWrapper {
            uint32_t* pixels;
            int width;
            int height;
        };

        struct Point {
            bool operator< (const Point& rhs) const {
                return x < rhs.x || (x == rhs.x && y < rhs.y);
            }
            float x, y;
        };

        struct PointI {
            bool operator< (const PointI& rhs) const {
                return x < rhs.x || (x == rhs.x && y < rhs.y);
            }
            int x, y;
        };

        typedef struct RgbColor
        {
            RgbColor() = default;
            RgbColor(const char8_t r8, const char8_t g8, const char8_t b8)
            { r=r8; g=g8; b=b8; }
            unsigned char r;
            unsigned char g;
            unsigned char b;
        } RgbColor;

        typedef struct HsvColor
        {
            unsigned char h;
            unsigned char s;
            unsigned char v;
        } HsvColor;

        static RgbColor HsvToRgb(const HsvColor hsv)
        {
            RgbColor rgb;

            if (hsv.s == 0)
            {
                rgb.r = hsv.v;
                rgb.g = hsv.v;
                rgb.b = hsv.v;
                return rgb;
            }

            const unsigned char region = hsv.h / 43;
            const unsigned char remainder = (hsv.h - (region * 43)) * 6;

            const unsigned char p = hsv.v * (255 - hsv.s) >> 8;
            const unsigned char q = hsv.v * (255 - (hsv.s * remainder >> 8)) >> 8;
            const unsigned char t = hsv.v * (255 - (hsv.s * (255 - remainder) >> 8)) >> 8;

            switch (region)
            {
                case 0:
                    rgb.r = hsv.v; rgb.g = t; rgb.b = p;
                    break;
                case 1:
                    rgb.r = q; rgb.g = hsv.v; rgb.b = p;
                    break;
                case 2:
                    rgb.r = p; rgb.g = hsv.v; rgb.b = t;
                    break;
                case 3:
                    rgb.r = p; rgb.g = q; rgb.b = hsv.v;
                    break;
                case 4:
                    rgb.r = t; rgb.g = p; rgb.b = hsv.v;
                    break;
                default:
                    rgb.r = hsv.v; rgb.g = p; rgb.b = q;
                    break;
            }

            return rgb;
        }

        static HsvColor RgbToHsv(const RgbColor rgb)
        {
            HsvColor hsv;

            const unsigned char rgbMin = rgb.r < rgb.g ? (rgb.r < rgb.b ? rgb.r : rgb.b) : (rgb.g < rgb.b ? rgb.g : rgb.b);
            const unsigned char rgbMax = rgb.r > rgb.g ? (rgb.r > rgb.b ? rgb.r : rgb.b) : (rgb.g > rgb.b ? rgb.g : rgb.b);

            hsv.v = rgbMax;
            if (hsv.v == 0)
            {
                hsv.h = 0;
                hsv.s = 0;
                return hsv;
            }

            hsv.s = 255 * static_cast<long>(rgbMax - rgbMin) / hsv.v;
            if (hsv.s == 0)
            {
                hsv.h = 0;
                return hsv;
            }

            if (rgbMax == rgb.r)
                hsv.h = 0 + 43 * (rgb.g - rgb.b) / (rgbMax - rgbMin);
            else if (rgbMax == rgb.g)
                hsv.h = 85 + 43 * (rgb.b - rgb.r) / (rgbMax - rgbMin);
            else
                hsv.h = 171 + 43 * (rgb.r - rgb.g) / (rgbMax - rgbMin);

            return hsv;
        }

        static RgbColor interpolate(const RgbColor a, const RgbColor b, float t)
        {
            // 0.0 <= t <= 1.0
            HsvColor ca = RgbToHsv(a);
            HsvColor cb = RgbToHsv(b);
            HsvColor final;

            auto interpolator = [](const int a, const int b, const float t) -> int {
                return a * (1 - t) + b * t;
            };

            final.h = interpolator(ca.h, cb.h, t);
            final.s = interpolator(ca.s, cb.s, t);
            final.v = interpolator(ca.v, cb.v, t);

            return HsvToRgb(final);
        }

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

        void AddParametricSurface(const ParametricSurfaceInfo& parametricSurfaceInfo) {
            _parametricSurfaces.emplace_back(parametricSurfaceInfo);
            _order.emplace_back(FuncType::PARAMSURFACE, _parametricSurfaces.size() - 1);
        }

        static void Plot(const int x,const int y,const Pixel color, const SurfaceWrapper& surface)
        {
            if (x < 0 || x >= surface.width || y < 0 || y >= surface.height) {
                return;
            }
            uint32_t* pixel = surface.pixels + (x + y * surface.width);
            *pixel = color.uint;
        }

#define OUTCODE(x,y) ((((x)<xmin)?1:(((x)>xmax)?2:0))+(((y)<ymin)?4:(((y)>ymax)?8:0)))
        static void Line(float x1, float y1, float x2, float y2, const Pixel color, const SurfaceWrapper& surface)
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
                *(surface.pixels + static_cast<int>(x1) + static_cast<int>(y1) * surface.width) = color.uint;
                x1 += dx, y1 += dy;
            }
        }

        static void DrawFunction(const SurfaceWrapper& surface, const FunctionInfo& info)
        {
            switch (info.plot) {
                case PlotType::PIXEL:
                    if (info.axis == Axis::X) {
                        for (int x = 0; x < surface.width; ++x)
                        {
                            Plot(x, static_cast<int>(info.function(x)), info.color, surface);
                        }
                    }
                    else {
                        for (int y = 0; y < surface.height; ++y)
                        {
                            Plot(static_cast<int>(info.function(y)), y, info.color, surface);
                        }
                    }
                    break;
                case PlotType::LINE:
                    // Collect points
                    {
                        std::vector<Point> points;
                        points.reserve(surface.width);
                        if (info.axis == Axis::X) {
                            for (int x = 0; x < surface.width; ++x) {
                                points.emplace_back(static_cast<float>(x), info.function(x));
                            }
                        }
                        else
                        {
                            for (int y = 0; y < surface.height; ++y) {
                                points.emplace_back(info.function(y) ,static_cast<float>(y) );
                            }
                        }


                        for (size_t i = 0; i < points.size() - 1; ++i) {
                            Point a = points[i];
                            Point b = points[i + 1];
                            Line(a.x, a.y, b.x, b.y, info.color, surface);
                        }
                    }
                    break;
            }
        }

        static void DrawSurface(const SurfaceWrapper& surface, const SurfaceInfo& info) {
            std::vector<float> values;
            values.reserve(surface.width * surface.height);

            float min = INFINITY;
            float max = -INFINITY;

            for (int y = 0; y < surface.height; ++y) {
                for (int x = 0; x < surface.width; ++x) {

                    float res = info.function(x, y);

                    min = std::min(min, res);
                    max = std::max(max, res);

                    values.emplace_back(res);
                }
            }

            Pixel lo { info.colorlo };
            Pixel hi { info.colorhi };

            for (int y = 0; y < surface.height; ++y) {
                for (int x = 0; x < surface.width; ++x) {
                    Pixel pixel = 0xffffffff;
                    const float normalized = std::clamp(values[x + y*surface.width] / (max - min), 0.f, 1.f);

                    const auto res = interpolate({lo.bytes[2], lo.bytes[1], lo.bytes[0]}, {hi.bytes[2], hi.bytes[1], hi.bytes[0]}, normalized);

                    pixel.bytes[3] = 0xff;
                    pixel.bytes[2] = res.r; //static_cast<char8_t>(std::lerp(lo.bytes[2], hi.bytes[2], normalized));
                    pixel.bytes[1] = res.g; //static_cast<char8_t>(std::lerp(lo.bytes[1], hi.bytes[1], normalized));
                    pixel.bytes[0] = res.b; //static_cast<char8_t>(std::lerp(lo.bytes[0], hi.bytes[0], normalized));
                    Plot(x, y, pixel, surface);
                }
            }
        }

        static void DrawEquation(const SurfaceWrapper& surface, const EquationInfo& info) {
            if (info.tStep == 0.f || (info.t0 > info.tMax && info.tStep > 0.f)) {
                return;
            }

            std::vector<Point> points;
            points.reserve(static_cast<size_t>((info.tMax - info.t0) / info.tStep));

            float t = info.t0;
            while (t < info.tMax) {

                auto res = info.equation(t);
                points.emplace_back(res.first, res.second);

                t += info.tStep;
            }

            switch (info.plot) {
                case PlotType::PIXEL:
                    for (Point point : points) {
                        Plot(static_cast<int>(point.x), static_cast<int>(point.y), info.color, surface);
                    }
                    break;
                case PlotType::LINE:
                    for (size_t i = 0; i < points.size() - 1; ++i) {
                        Point a = points[i];
                        Point b = points[i+1];
                        Line(a.x, a.y, b.x, b.y, info.color, surface);
                    }
                    break;
            }
        }

        static void DrawParametricSurface(const SurfaceWrapper& surface, const ParametricSurfaceInfo& info) {
            if (info.tStep == 0.f || (info.t0 > info.tMax && info.tStep > 0.f)) {
                return;
            }
            if (info.sStep == 0.f || (info.s0 > info.sMax && info.sStep > 0.f)) {
                return;
            }

            std::map<PointI, float> points;

            float min = INFINITY;
            float max = -INFINITY;

            float t = info.t0;
            float s = info.s0;

            while (t < info.tMax) {
                while (s < info.sMax) {
                    auto res = info.function(t, s);

                    const auto x = static_cast<int>(std::get<0>(res));
                    const auto y = static_cast<int>(std::get<1>(res));
                    const auto z = std::get<2>(res);

                    min = std::min(min, z);
                    max = std::max(max, z);

                    if (points.contains({x, y})) {
                        points[{x, y}] = std::max( points[{x, y}], z);
                    }
                    else {
                        points.emplace(PointI(x, y), z);
                    }

                    s += info.sStep;
                }
                s = info.s0;
                t += info.tStep;
            }

            for (auto pair : points) {

                const auto point = pair.first;
                const auto height = pair.second;

                Pixel pixel = 0xffffffff;

                Pixel lo { info.colorlo };
                Pixel hi { info.colorhi };

                const float normalized = std::clamp(height / (max - min), 0.f, 1.f);

                auto res = interpolate({lo.bytes[2], lo.bytes[1], lo.bytes[0]}, {hi.bytes[2], hi.bytes[1], hi.bytes[0]}, normalized);

                pixel.bytes[3] = 0xff;
                pixel.bytes[2] = res.r;
                pixel.bytes[1] = res.g;
                pixel.bytes[0] = res.b;
                Plot(point.x, point.y, pixel, surface);
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
                    case FuncType::PARAMSURFACE:
                        DrawParametricSurface(surface, _parametricSurfaces[pair.second]);
                        break;
                    default: ;
                }
            }
        }
    };
}