#pragma once

#include "Console.h"
#include <boost/serialization/access.hpp>
#include <boost/serialization/base_object.hpp>
#include <boost/serialization/deque.hpp>
#include <boost/serialization/queue.hpp>
#include <boost/serialization/set.hpp>
#include <boost/serialization/shared_ptr.hpp>
#include <boost/serialization/utility.hpp>
#include <boost/serialization/vector.hpp>
#include <functional>
#include <memory>
#include <queue>
#include <set>
#include <vector>

namespace Hilltop {
    namespace Game {
        using Console::ConsoleColor;

        float scale(float value, float fromLow, float fromHigh, float toLow, float toHigh);

        class Vector2 {
        private:
            friend class boost::serialization::access;
            template<class Archive>
            void serialize(Archive &ar, const unsigned int version) {
                ar & X;
                ar & Y;
            }

        public:
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
        private:
            friend class boost::serialization::access;
            template<class Archive>
            void serialize(Archive &ar, const unsigned int version) {
                ar & position;
                ar & direction;
                ar & gravityMult;
                ar & groundHog;
                ar & hasHit;
                ar & hasExpired;
                ar & entityAge;
                ar & maxEntityAge;
                ar & physicsSpeed;
            }

        protected:
            Entity();

        public:
            Vector2 position = { -1.0f, -1.0f };
            Vector2 direction = { 0.0f, 0.0f };
            float gravityMult = 1.0f;

            bool groundHog = false;
            bool hasHit = false;
            bool hasExpired = false;
            int entityAge = 0;
            int maxEntityAge = -1;
            int physicsSpeed = 1;

            virtual ~Entity();
            static std::shared_ptr<Entity> create();

            virtual void onTick(TankMatch *match);
            virtual void onDraw(TankMatch *match, Console::DoublePixelBufferedConsole &console);
            virtual void onDirectDraw(TankMatch *match, Console::BufferedConsole &console);
            virtual void onHit(TankMatch *match);
            virtual void onExpire(TankMatch *match);
        };


        class BotAttempt : public Entity {
        private:
            friend class boost::serialization::access;
            template<class Archive>
            void serialize(Archive &ar, const unsigned int version) {
                ar & boost::serialization::base_object<Entity>(*this);
                ar & color;
                ar & angle;
                ar & power;
            }

        protected:
            BotAttempt();

        public:
            static bool enableDebug;
            ConsoleColor color = ConsoleColor::RED;

            int angle;
            int power;

            static std::shared_ptr<BotAttempt> create();

            virtual void onDraw(TankMatch *match, Console::DoublePixelBufferedConsole &console) override;
        };


        class SimpleRocket : public Entity {
        private:
            friend class boost::serialization::access;
            template<class Archive>
            friend inline void load_construct_data(Archive &ar, SimpleRocket *t, const unsigned int);
            template<class Archive>
            void serialize(Archive &ar, const unsigned int version) {
                ar & boost::serialization::base_object<Entity>(*this);
                ar & color;
                ar & explosionSize;
                ar & destroyLand;
                ar & createLand;
            }

        protected:
            SimpleRocket(ConsoleColor color);

        public:
            ConsoleColor color;

            int explosionSize = 5;

            bool destroyLand = true;
            bool createLand = false;

            static std::shared_ptr<SimpleRocket> create(ConsoleColor color);

            virtual void onDraw(TankMatch *match, Console::DoublePixelBufferedConsole &console) override;
            virtual void onHit(TankMatch *match) override;
        };

        template<class Archive>
        inline void load_construct_data(Archive &ar, SimpleRocket *t, const unsigned int) {
            ::new(t) SimpleRocket(ConsoleColor());
        }


        class SimpleTrailedRocket : public SimpleRocket {
        private:
            friend class boost::serialization::access;
            template<class Archive>
            friend inline void load_construct_data(Archive &ar, SimpleTrailedRocket *t, const unsigned int);
            template<class Archive>
            void serialize(Archive &ar, const unsigned int version) {
                ar & boost::serialization::base_object<SimpleRocket>(*this);
                ar & trailColor;
                ar & trailTime;
            }

        protected:
            SimpleTrailedRocket(ConsoleColor color, ConsoleColor trailColor, int trailTime);

        public:
            ConsoleColor trailColor;
            int trailTime;

            static std::shared_ptr<SimpleTrailedRocket> create(ConsoleColor color, ConsoleColor trailColor, int trailTime);

            virtual void onTick(TankMatch *match) override;
        };

        template<class Archive>
        inline void load_construct_data(Archive &ar, SimpleTrailedRocket *t, const unsigned int) {
            ::new(t) SimpleTrailedRocket(ConsoleColor(), ConsoleColor(), 0);
        }


        class RocketTrail : public Entity {
        private:
            friend class boost::serialization::access;
            template<class Archive>
            friend inline void load_construct_data(Archive &ar, RocketTrail *t, const unsigned int);
            template<class Archive>
            void serialize(Archive &ar, const unsigned int version) {
                ar & boost::serialization::base_object<Entity>(*this);
                ar & color;
            }

        protected:
            RocketTrail(int maxAge, ConsoleColor color);

        public:
            ConsoleColor color;

            static std::shared_ptr<RocketTrail> create(int maxAge, ConsoleColor color);

            virtual void onDraw(TankMatch *match, Console::DoublePixelBufferedConsole &console) override;
        };

        template<class Archive>
        inline void load_construct_data(Archive &ar, RocketTrail *t, const unsigned int) {
            ::new(t) RocketTrail(0, ConsoleColor());
        }



        class Tank;

        class Explosion : public Entity {
        private:
            friend class boost::serialization::access;
            template<class Archive>
            friend inline void load_construct_data(Archive &ar, Explosion *t, const unsigned int);
            template<class Archive>
            void serialize(Archive &ar, const unsigned int version) {
                ar & boost::serialization::base_object<Entity>(*this);
                ar & coreSize;
                ar & tanksHit;
            }

        protected:
            Explosion(int size);

            static const int ticksBetween = 2;

            void destroyLand(TankMatch *match);
            void createLand(TankMatch *match);

        public:
            const int size;
            int coreSize;

            bool willDestroyLand = false;
            bool willCreateLand = false;

            std::set<std::shared_ptr<Tank>> tanksHit;
            int calcDamage(Vector2 point);
            void hitTanks(TankMatch *match);

            static std::shared_ptr<Explosion> create(int size);

            virtual void onTick(TankMatch *match) override;
            virtual void onDraw(TankMatch *match, Console::DoublePixelBufferedConsole &console) override;
        };

        template<class Archive>
        inline void save_construct_data(Archive &ar, const Explosion *t, const unsigned int) {
            ar << t->size;
        }

        template<class Archive>
        void load_construct_data(Archive &ar, Explosion *t, const unsigned int) {
            int size;
            ar >> size;
            ::new(t) Explosion(size);
        }


        class Tracer : public Entity {
        private:
            friend class boost::serialization::access;
            template<class Archive>
            void serialize(Archive &ar, const unsigned int version) {
                ar & boost::serialization::base_object<Entity>(*this);
                ar & text;
            }

        protected:
            Tracer();

        public:
            static const int DURATION = 30;

            std::string text;

            static std::shared_ptr<Tracer> create();

            virtual void onTick(TankMatch *match) override;
            virtual void onDraw(TankMatch *match, Console::DoublePixelBufferedConsole &console) override;
            virtual void onHit(TankMatch *match) override;
        };


        class TankWheel : public Entity {
        private:
            friend class boost::serialization::access;
            template<class Archive>
            void serialize(Archive &ar, const unsigned int version) {
                ar & boost::serialization::base_object<Entity>(*this);
            }

        protected:
            TankWheel();

        public:
            static bool enableDebug;

            static std::shared_ptr<TankWheel> create();

            virtual void onDraw(TankMatch *match, Console::DoublePixelBufferedConsole &console) override;
        };


        class Tank : public Entity {
        private:
            friend class boost::serialization::access;
            template<class Archive>
            friend inline void load_construct_data(Archive &ar, Tank *t, const unsigned int);
            template<class Archive>
            void serialize(Archive &ar, const unsigned int version) {
                ar & boost::serialization::base_object<Entity>(*this);
                ar & color;
                ar & wheels;
                ar & alive;
                ar & angle;
                ar & power;
                ar & maxHealth;
                ar & health;
                ar & maxArmor;
                ar & armor;
                ar & damage;
            }

        protected:
            Tank(ConsoleColor color);

        public:
            static const ConsoleColor DEAD_COLOR = ConsoleColor::DARK_GRAY;

            ConsoleColor color;
            ConsoleColor getActualColor();

            std::shared_ptr<Entity> wheels[5];

            bool alive = true;
            int angle = 45;
            int power = 50;
            int maxHealth = 100;
            int health = 100;
            int maxArmor = 0;
            int armor = 0;
            float damage = 1.0f;

            Vector2 getBarrelBase();
            Vector2 getBarrelEnd();
            static Vector2 getBarrelEnd(int angle);
            Vector2 getProjectileBase();
            static Vector2 getProjectileBase(Vector2 barrelEnd);
            static Vector2 getProjectileBase(int angle);

            static Vector2 calcTrajectory(int angle, int power);
            Vector2 calcTrajectory();
            std::vector<Vector2> getPixels();
            bool testCollision(Vector2 position);

            static std::shared_ptr<Tank> create(ConsoleColor color);
            void initWheels(TankMatch &match);

            virtual void onTick(TankMatch *match) override;
            virtual void onDraw(TankMatch *match, Console::DoublePixelBufferedConsole &console) override;

            void drawReticle(TankMatch *match, Console::DoublePixelBufferedConsole &console);

            bool canMove(TankMatch *match, int direction);
            void doMove(TankMatch *match, int direction);

            void dealDamage(TankMatch *match, int damage);
            void die(TankMatch *match);
        };

        template<class Archive>
        inline void load_construct_data(Archive &ar, Tank *t, const unsigned int) {
            ::new(t) Tank(ConsoleColor());
        }


        class Drop : public Entity {
        private:
            friend class boost::serialization::access;
            template<class Archive>
            void serialize(Archive &ar, const unsigned int version) {
                ar & boost::serialization::base_object<Entity>(*this);
                ar & color;
                ar & ch;
            }

        protected:
            Drop();

        public:
            ConsoleColor color = ConsoleColor::BLACK;
            wchar_t ch = L' ';

            int getTopRow();

            virtual void onTick(TankMatch *match) override;
            virtual void onDirectDraw(TankMatch *match, Console::BufferedConsole &console) override;

            virtual void handleTank(TankMatch *match, Tank &tank);
        };


        class HealthDrop : public Drop {
        private:
            friend class boost::serialization::access;
            template<class Archive>
            void serialize(Archive &ar, const unsigned int version) {
                ar & boost::serialization::base_object<Drop>(*this);
            }

        protected:
            HealthDrop();

        public:
            static const int HEALTH = 50;

            static std::shared_ptr<HealthDrop> create();

            virtual void handleTank(TankMatch *match, Tank &tank) override;
        };


        class ArmorDrop : public Drop {
        private:
            friend class boost::serialization::access;
            template<class Archive>
            void serialize(Archive &ar, const unsigned int version) {
                ar & boost::serialization::base_object<Drop>(*this);
            }

        protected:
            ArmorDrop();

        public:
            static const int ARMOR = 25;

            static std::shared_ptr<ArmorDrop> create();

            virtual void handleTank(TankMatch *match, Tank &tank) override;
        };


        class WeaponDrop : public Drop {
        private:
            friend class boost::serialization::access;
            template<class Archive>
            void serialize(Archive &ar, const unsigned int version) {
                ar & boost::serialization::base_object<Drop>(*this);
            }

        protected:
            WeaponDrop();

        public:
            static const int WEAPONS = 10;

            static std::shared_ptr<WeaponDrop> create();

            virtual void handleTank(TankMatch *match, Tank &tank) override;
        };



        class Weapon {
        private:
            friend class boost::serialization::access;
            template<class Archive>
            void serialize(Archive &ar, const unsigned int version) {
                ar & name;
                ar & icon;
            }

        public:
            static const std::string INVALID_NAME;

            std::string name = INVALID_NAME;

            Console::BufferedConsole::pixel_t icon[2];

            virtual void fire(TankMatch &match, int playerNumber);
        };


        class RocketWeapon : public Weapon {
        private:
            friend class boost::serialization::access;
            template<class Archive>
            friend inline void load_construct_data(Archive &ar, RocketWeapon *t, const unsigned int);
            template<class Archive>
            void serialize(Archive &ar, const unsigned int version) {
                ar & boost::serialization::base_object<Weapon>(*this);
            }

        protected:
            virtual std::shared_ptr<SimpleRocket> createRocket(Vector2 position, Vector2 direction);
            
        public:
            RocketWeapon(int numRockets);

            const int numRockets;

            virtual void fire(TankMatch &match, int playerNumber) override;
        };

        template<class Archive>
        inline void save_construct_data(Archive &ar, const RocketWeapon *t, const unsigned int) {
            ar << t->numRockets;
        }

        template<class Archive>
        inline void load_construct_data(Archive &ar, RocketWeapon *t, const unsigned int) {
            int numRockets;
            ar >> numRockets;
            ::new(t) RocketWeapon(numRockets);
        }


        class DirtRocketWeapon : public RocketWeapon {
        private:
            friend class boost::serialization::access;
            template<class Archive>
            friend inline void load_construct_data(Archive &ar, DirtRocketWeapon *t, const unsigned int);
            template<class Archive>
            void serialize(Archive &ar, const unsigned int version) {
                ar & boost::serialization::base_object<RocketWeapon>(*this);
            }

        protected:
            virtual std::shared_ptr<SimpleRocket> createRocket(Vector2 position, Vector2 direction) override;

        public:
            DirtRocketWeapon(int numRockets);
        };

        template<class Archive>
        inline void load_construct_data(Archive &ar, DirtRocketWeapon *t, const unsigned int) {
            ::new(t) DirtRocketWeapon(0);
        }



        class TankController : public std::enable_shared_from_this<TankController> {
        private:
            friend class boost::serialization::access;
            template<class Archive>
            void serialize(Archive &ar, const unsigned int version) {
                ar & botAttempts;
                ar & botTargetTank;
                ar & botTarget;
                ar & botTargetAngle;
                ar & botTargetPower;
                ar & botStepsDone;
                ar & botLastStepTick;
                ar & tank;
                ar & team;
                ar & isHuman;
                ar & currentWeapon;
                ar & movesPerTurn;
                ar & movesLeft;
                ar & weapons;
            }

        protected:
            TankController();

        public:
            static const int BOT_ATTEMPT_SPEED = 5;
            static const int BOT_MAX_ATTEMPT_TIME = 80;
            static constexpr int RANDOM_ATTEMPTS_BY_BOT_DIFFICULTY[] = {
                10, 30, 80
            };
            static const int BOT_STEPS = 6;
            static const int BOT_TICKS_BETWEEN_STEPS = 4;
            std::vector<std::shared_ptr<BotAttempt>> botAttempts;
            std::shared_ptr<Tank> botTargetTank;
            Vector2 botTarget;
            int botTargetAngle = -1;
            int botTargetPower = -1;
            int botStepsDone;
            int botLastStepTick;

            std::shared_ptr<Tank> tank;
            int team = 1;
            bool isHuman = true;
            int currentWeapon = 0;
            int movesPerTurn = 25;
            int movesLeft = movesPerTurn;

            static std::shared_ptr<TankController> create();

            std::vector<std::pair<std::shared_ptr<Weapon>, int>> weapons;
            void addWeapon(std::shared_ptr<Weapon> weapon, int amount);
            void addRandomWeapon();
            int getWeaponCount();

            static bool applyAI(TankMatch *match, TankController &player);
        };



        class TankMatch {
        public:
            enum LandType : unsigned char {
                AIR = 0,
                GRASS,
                DIRT,
                NOTHING,
                NUM_LAND_TYPES,
            };

            friend class boost::serialization::access;
            template<class Archive>
            void save(Archive &ar, const unsigned int version) const {
                ar & entities;
                ar & entityChanges;
                ar & recentUpdateResult;
                ar & players;
                ar & currentPlayer;
                ar & gravity;
                ar & tickNumber;
                ar & isAiming;
                ar & gameOver;
                ar & firingMode;
                
                if (map.size() > 0) {
                    LandType lastType = map[0];
                    size_t spanLength = 1;
                    for (int i = 1; i < map.size(); i++) {
                        LandType type = map[i];
                        if (type != lastType) {
                            ar & lastType;
                            ar & spanLength;
                            lastType = type;
                            spanLength = 1;
                        } else {
                            spanLength++;
                        }
                    }
                    ar & lastType;
                    ar & spanLength;
                }
            }
            template<class Archive>
            void load(Archive &ar, const unsigned int version) {
                ar & entities;
                ar & entityChanges;
                ar & recentUpdateResult;
                ar & players;
                ar & currentPlayer;
                ar & gravity;
                ar & tickNumber;
                ar & isAiming;
                ar & gameOver;
                ar & firingMode;

                LandType type;
                size_t spanLength = 0;
                for (int i = 0; i < map.size(); i++) {
                    if (spanLength == 0) {
                        ar & type;
                        ar & spanLength;
                    }
                    map[i] = type;
                    spanLength--;
                }
            }
            BOOST_SERIALIZATION_SPLIT_MEMBER()

            std::vector<LandType> map;

            std::vector<std::shared_ptr<Entity>> entities;
            std::queue<std::pair<bool, std::shared_ptr<Entity>>> entityChanges;

            static const int RECENT_UPDATE_COUNT = 10;
            bool recentUpdateResult[10] = {};

            static const int AIM_RETICLE_TIME = 6;
            static const int LAND_PHYSICS_EVERY_TICKS = 3;

            bool doEntityTick();
            bool doLandPhysics();

        public:
            enum FiringMode {
                FIRE_SOLO,
                FIRE_AS_TEAM,
                FIRE_EVERYTHING,
            };

            static const int UNLIMITED_WEAPON_THRESHOLD = 99;

            static const int DEFAULT_MATCH_WIDTH = 180;
            static const int DEFAULT_MATCH_HEIGHT = 90;

            const unsigned short width, height;
            Console::DoublePixelBufferedConsole canvas;

            std::vector<std::shared_ptr<TankController>> players;
            int currentPlayer = 0;

            Vector2 gravity = { 0.15f, 0.0f };
            bool updateMattered = false;
            uint64_t tickNumber = 0;
            bool isAiming = true;
            bool gameOver = false;
            bool shownGameOver = false;
            unsigned short lowestAir;
            unsigned short highestLand;

            FiringMode firingMode = FIRE_AS_TEAM;

            static std::vector<std::shared_ptr<Weapon>> weapons;
            static void initalizeWeapons();

            TankMatch();
            TankMatch(unsigned short width, unsigned short height);

            LandType get(int x, int y);
            void set(int x, int y, LandType type);

            void addEntity(Entity &entity);
            void removeEntity(Entity &entity);

            void buildMap(std::function<float(float)> generator);
            void arrangeTanks();
            std::pair<bool, Vector2> checkForHit(const Vector2 from, const Vector2 to, bool groundHog = false);
            void doAirdrop();

            void draw(Console::BufferedConsole &console);

            void tick();
            bool recentUpdatesMattered();
            void fire(int playerNumber);
            void fire();

            int getNextPlayer();
        };

        template<class Archive>
        inline void save_construct_data(Archive &ar, const TankMatch *t, const unsigned int) {
            ar << t->width;
            ar << t->height;
        }

        template<class Archive>
        inline void load_construct_data(Archive &ar, TankMatch *t, const unsigned int) {
            unsigned short width, height;
            ar >> width;
            ar >> height;
            ::new(t) TankMatch(width, height);
        }
    }
}
