#pragma once

#include <vector>

namespace Hilltop {
    namespace Console {
        class DoublePixelBufferedConsole;
    }

    namespace Game {

        struct Vector2 {
            float X, Y;
        };


        class Projectile {
        public:
            virtual ~Projectile();
        };


        enum LandType : unsigned char {
            AIR = 0,
            GRASS = 1,
            DIRT = 2,
            NUM_LAND_TYPES
        };

        class TankMatch {
        private:
            std::vector<LandType> map;

            LandType get(unsigned short x, unsigned short y);
            void set(unsigned short x, unsigned short y, LandType type);

        public:
            const unsigned short width, height;

            TankMatch(unsigned short width, unsigned short height);

            void buildMap();
            
            void draw(Console::DoublePixelBufferedConsole &console);

            void doTick(uint64_t tickNumber);
        };
    }
}
