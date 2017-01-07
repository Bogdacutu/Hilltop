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



        class Entity : public std::enable_shared_from_this<Entity> {
        protected:
            Entity();

        public:
            Vector2 position = { -1.0f, -1.0f };
            Vector2 direction = { 0.0f, 0.0f };
            float gravityMult = 1.0f;

            bool groundHog = false;
            bool hasHit = false;
            int entityAge = 0;
            int maxEntityAge = -1;

            virtual ~Entity();
            static std::shared_ptr<Entity> create();

            virtual void onTick(TankMatch *match);
            virtual void onDraw(TankMatch *match, Console::DoublePixelBufferedConsole &console);
            virtual void onHit(TankMatch *match);
            virtual void onExpire(TankMatch *match);
        };


        class SimpleRocket : public Entity {
        protected:
            SimpleRocket(ConsoleColor color);

        public:
            ConsoleColor color;

            bool destroyLand = true;
            bool createLand = false;

            static std::shared_ptr<SimpleRocket> create(ConsoleColor color);

            virtual void onDraw(TankMatch *match, Console::DoublePixelBufferedConsole &console) override;
            virtual void onHit(TankMatch *match) override;
        };


        class SimpleTrailedRocket final : public SimpleRocket {
        protected:
            SimpleTrailedRocket(ConsoleColor color, ConsoleColor trailColor, int trailTime);

        public:
            ConsoleColor trailColor;
            int trailTime;

            static std::shared_ptr<SimpleTrailedRocket> create(ConsoleColor color, ConsoleColor trailColor, int trailTime);

            virtual void onTick(TankMatch *match) override;
        };


        class RocketTrail final : public Entity {
        protected:
            RocketTrail(int maxAge, ConsoleColor color);

        public:
            ConsoleColor color;

            static std::shared_ptr<RocketTrail> create(int maxAge, ConsoleColor color);

            virtual void onDraw(TankMatch *match, Console::DoublePixelBufferedConsole &console) override;
        };


        class Explosion final : public Entity {
        protected:
            Explosion(int size, int damage);

            const int ticksBetween = 2;

            void destroyLand(TankMatch *match);
            void createLand(TankMatch *match);

        public:
            const int size;
            int coreSize;
            const int damage;

            bool willDestroyLand = false;
            bool willCreateLand = false;

            static std::shared_ptr<Explosion> create(int size, int damage);

            virtual void onTick(TankMatch *match) override;
            virtual void onDraw(TankMatch *match, Console::DoublePixelBufferedConsole &console) override;
        };


        class Tracer : public Entity {
        protected:
            Tracer();

        public:
            static const int tracerDuration = 30;

            std::string text;

            static std::shared_ptr<Tracer> create();

            virtual void onTick(TankMatch *match) override;
            virtual void onDraw(TankMatch *match, Console::DoublePixelBufferedConsole &console) override;
            virtual void onHit(TankMatch *match) override;
        };


        class TankWheel final : public Entity {
        protected:
            TankWheel();

        public:
            static bool enableDebug;

            static std::shared_ptr<TankWheel> create();

            virtual void onDraw(TankMatch *match, Console::DoublePixelBufferedConsole &console) override;
        };


        class Tank final : public Entity {
        protected:
            Tank(ConsoleColor color, ConsoleColor barrelColor);

        public:
            ConsoleColor color;
            ConsoleColor barrelColor;

            std::shared_ptr<Entity> wheels[5];

            int angle = 45;
            int power = 50;

            Vector2 getBarrelBase();
            Vector2 getBarrelEnd();
            Vector2 getProjectileBase();

            static Vector2 calcTrajectory(int angle, int power);
            Vector2 calcTrajectory();

            static std::shared_ptr<Tank> create(ConsoleColor color);
            static std::shared_ptr<Tank> create(ConsoleColor color, ConsoleColor barrelColor);
            void initWheels(TankMatch &match);

            virtual void onTick(TankMatch *match) override;
            virtual void onDraw(TankMatch *match, Console::DoublePixelBufferedConsole &console) override;

            void drawReticle(TankMatch *match, Console::DoublePixelBufferedConsole &console);
        };



        class Weapon {
        public:
            static const std::string INVALID_NAME;

            std::string name = INVALID_NAME;

            Console::BufferedConsole::pixel_t icon[2];

            virtual void fire(TankMatch &match);
        };


        class RocketWeapon : public Weapon {
        protected:
            virtual std::shared_ptr<SimpleRocket> createRocket(Vector2 position, Vector2 direction);
            
        public:
            RocketWeapon(int numRockets);

            const int numRockets;

            virtual void fire(TankMatch &match) override;
        };


        class DirtRocketWeapon final : public RocketWeapon {
        protected:
            virtual std::shared_ptr<SimpleRocket> createRocket(Vector2 position, Vector2 direction) override;

        public:
            DirtRocketWeapon(int numRockets);
        };



        class TankController : public std::enable_shared_from_this<TankController> {
        protected:
            TankController();

        public:
            std::shared_ptr<Tank> tank;
            int team = 1;
            bool isHuman = true;
            std::vector<std::pair<std::shared_ptr<Weapon>, int>> weapons;
            int currentWeapon = 0;
            int movesLeft = 0;

            int aiDifficulty = 0;
            Vector2 lastHit;
            bool hasLastHit = false;

            static std::shared_ptr<TankController> create();

            static void applyAI(TankController &player);
        };


        class LastHitTracer final : public Tracer {
        protected:
            LastHitTracer(TankController &player);

        public:
            const std::shared_ptr<TankController> player;

            static std::shared_ptr<LastHitTracer> create(TankController &player);

            virtual void onTick(TankMatch *match) override;
            virtual void onDraw(TankMatch *match, Console::DoublePixelBufferedConsole &console) override;
            virtual void onHit(TankMatch *match) override;
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

            std::vector<std::shared_ptr<Entity>> entities;
            std::queue<std::pair<bool, std::shared_ptr<Entity>>> entityChanges;

            static const int recentUpdateCount = 10;
            bool recentUpdateResult[10] = {};

            static const int aimReticleTime = 6;

            bool doEntityTick();
            bool doLandPhysics();

        public:
            const unsigned short width, height;
            Console::DoublePixelBufferedConsole canvas;

            std::vector<std::shared_ptr<TankController>> players;
            int currentPlayer = 0;

            Vector2 gravity = { 0.15f, 0.0f };
            bool updateMattered = false;
            uint64_t tickNumber = 0;
            bool isAiming = true;

            static std::vector<std::shared_ptr<Weapon>> weapons;
            static void initalizeWeapons();

            TankMatch(unsigned short width, unsigned short height);

            LandType get(int x, int y);
            void set(int x, int y, LandType type);

            void addEntity(Entity &entity);
            void removeEntity(Entity &entity);

            void buildMap(std::function<float(float)> generator);
            void arrangeTanks();
            std::pair<bool, Vector2> checkForHit(const Vector2 from, const Vector2 to, bool groundHog = false);
            
            void draw(Console::Console &console);

            void doTick();
            bool recentUpdatesMattered();
            void fire();

            int getNextPlayer();
        };
    }
}
