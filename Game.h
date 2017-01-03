#pragma once

#include "Console.h"
#include <functional>
#include <memory>
#include <queue>
#include <vector>

namespace Hilltop {
    namespace Game {
        using Console::ConsoleColor;

        float scale(float value, float fromLow, float fromHigh, float toLow, float toHigh);

        struct Vector2 {
            float X = 0;
            float Y = 0;

            Vector2() {}
            Vector2(float X, float Y) : X(X), Y(Y) {}

            Vector2 operator+(const Vector2 &other) const {
                return Vector2(X + other.X, Y + other.Y);
            }

            Vector2 operator-(const Vector2 &other) const {
                return Vector2(X - other.X, Y - other.Y);
            }

            Vector2 operator*(float other) const {
                return Vector2(X * other, Y * other);
            }

            bool operator==(const Vector2 &other) const {
                return X == other.X && Y == other.Y;
            }

            bool operator!=(const Vector2 &other) const {
                return X != other.X || Y != other.Y;
            }

            Vector2 floor() const {
                return Vector2(std::floor(X), std::floor(Y));
            }

            Vector2 round() const {
                return Vector2(std::round(X), std::round(Y));
            }

            Vector2 ceil() const {
                return Vector2(std::ceil(X), std::ceil(Y));
            }

            Vector2 abs() const {
                return Vector2(std::abs(X), std::abs(Y));
            }
        };

        float distance(const Vector2 from, const Vector2 to);

        void foreachPixel(const Vector2 from, const Vector2 to, std::function<bool(Vector2)> handler);


        class TankMatch;


        class Projectile : public std::enable_shared_from_this<Projectile> {
        protected:
            Projectile();

        public:
            Vector2 position;
            Vector2 direction;
            Vector2 gravity = { 0.15, 0 };

            bool groundHog = false;
            bool hasHit = false;

            virtual ~Projectile();

            virtual void onTick(TankMatch *match);
            virtual void onDraw(TankMatch *match, Console::DoublePixelBufferedConsole &console);
            virtual void onHit(TankMatch *match);
        };


        class SimpleMissle : public Projectile {
        protected:
            SimpleMissle(ConsoleColor color);

        public:
            ConsoleColor color;

            static std::shared_ptr<SimpleMissle> create(ConsoleColor color);

            virtual void onDraw(TankMatch *match, Console::DoublePixelBufferedConsole &console) override;
            virtual void onHit(TankMatch *match) override;
        };

        
        class SimpleTrailedMissile final : public SimpleMissle {
        protected:
            SimpleTrailedMissile(ConsoleColor color, ConsoleColor trailColor, int trailTime);

        public:
            ConsoleColor trailColor;
            int trailTime;

            static std::shared_ptr<SimpleTrailedMissile> create(ConsoleColor color, ConsoleColor trailColor, int trailTime);

            virtual void onTick(TankMatch *match) override;
        };


        class MissleTrail final : public Projectile {
        protected:
            MissleTrail(int maxAge, ConsoleColor color);

        public:
            const int maxAge;
            int age = 0;
            ConsoleColor color;

            static std::shared_ptr<MissleTrail> create(int maxAge, ConsoleColor color);

            virtual void onTick(TankMatch *match) override;
            virtual void onDraw(TankMatch *match, Console::DoublePixelBufferedConsole &console) override;
        };


        class Explosion : public Projectile {
        protected:
            Explosion(int size, int damage);

            void destroyLand(TankMatch *match);

            bool firstTick = true;
            const int ticksBetween = 1;
            int ticksLeft = ticksBetween;

        public:
            const int size;
            int coreSize;
            const int damage;

            bool willDestroyLand = false;

            static std::shared_ptr<Explosion> create(int size, int damage);

            virtual void onTick(TankMatch *match) override;
            virtual void onDraw(TankMatch *match, Console::DoublePixelBufferedConsole &console) override;
        };



        enum LandType : unsigned char {
            AIR = 0,
            GRASS,
            DIRT,
            NOTHING,
            NUM_LAND_TYPES,
        };

        class TankMatch {
        private:
            std::vector<LandType> map;

            std::vector<std::shared_ptr<Projectile>> projectiles;
            std::queue<std::pair<bool, std::shared_ptr<Projectile>>> projectileChanges;

            const int timeBetweenMissles = 3;
            int timeSinceLast = 0;

            void doProjectileTick();
            Vector2 calcTrajectory(int angle, int power);

        public:
            const unsigned short width, height;

            TankMatch(unsigned short width, unsigned short height);

            LandType get(int x, int y);
            void set(int x, int y, LandType type);

            void addProjectile(Projectile &projectile);
            void removeProjectile(Projectile &projectile);

            void buildMap(std::function<float(float)> generator);
            std::pair<bool, Vector2> checkForHit(const Vector2 from, const Vector2 to, bool groundHog = false);
            
            void draw(Console::DoublePixelBufferedConsole &console);

            void doTick(uint64_t tickNumber);
        };
    }
}
