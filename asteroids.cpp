#define OLC_PGE_APPLICATION
#include "olcPixelGameEngine.h"
#include <deque>

const float PI = 3.14159;

const float WIDTH = 400;
const float HEIGHT = 400;

float toRad(float value)
{
	return 1.0f / tanf(value * 0.5f / 180.0f * 3.14159f);
}

float square(float a)
{
    return a*a;
}

class Object
{
protected:
    olc::vf2d pos;
    float angle;
    float speed;
    float size;

public:
    Object(olc::vf2d pos)
    {
        this->pos = pos;
        this->angle = 0;
        this->speed = 0;
        this->size = 10;
    }

    Object(olc::vf2d pos, float angle, float speed)
    {
        this->pos = pos;
        this->angle = angle;
        this->speed = speed;
        this->size = 10;
    }

    Object(olc::vf2d pos, float angle, float speed, float size)
    {
        this->pos = pos;
        this->angle = angle;
        this->speed = speed;
        this->size = size;
    }

    void moveTo(olc::vf2d newPos)
    {
        pos = newPos;
    }

    void moveBy(olc::vf2d newPosDelta)
    {
        pos += newPosDelta;
    }

    void rotateBy(float newAngleDelta)
    {
        angle += newAngleDelta;
    }

    olc::vf2d getPos()
    {
        return this->pos;
    }

    float getAngle()
    {
        return this->angle;
    }

    float getSize()
    {
        return this->size;
    }

    float getSpeed()
    {
        return this->speed;
    }

    void addSpeed(float toAdd)
    {
        this->speed = std::fmax(std::fmin((this->speed+toAdd), 5), 0);
    }
    
    void moveBySpeed()
    {
        this->pos.x = std::fmod(std::sin(this->angle)*this->speed+this->pos.x, HEIGHT);
        this->pos.y = std::fmod(std::cos(this->angle)*this->speed+this->pos.y, WIDTH);
        if(this->pos.x < 0)
            this->pos.x = this->pos.x+400;
        if(this->pos.y < 0)
            this->pos.y = this->pos.y+400;
    }

    float distanceToObject(Object other)
    {
        float distX = this->pos.x - other.getPos().x;
        float distY = this->pos.y - other.getPos().y;

        return (std::sqrt(distX*distX+distY*distY) - this->size);
    }
};

class Bullet : public Object
{
public:
    Bullet(olc::vf2d pos, float angle) : Object(pos, angle, 10.0f){}

float lifetime = 100;

    std::vector<olc::vf2d> getDrawingPoints()
    {
        olc::vf2d point1 = {this->pos.x, this->pos.y};
        olc::vf2d point2 = {std::sin(this->angle)*5+this->pos.x, std::cos(this->angle)*5+this->pos.y};

        std::vector<olc::vf2d> points = {point1, point2};
        return points;
    }
};

class Asteroid : public Object
{
public:
    Asteroid(olc::vf2d pos, float angle, float speed, float size) : Object(pos, angle, speed, size){}

    std::pair<float, Bullet*> getClosestBullet(std::deque<Bullet*>* bullets)
    {
        std::vector<std::pair<float, Bullet*>> distances;

        for (Bullet* bullet : *bullets)
        {
            distances.push_back(std::make_pair(this->distanceToObject(*bullet), bullet));
        }


        std::sort(distances.begin(), distances.end());
        return distances[0];
    }
};

class Ship : public Object
{
public:
    Ship(std::deque<Bullet*>* bullets) : Object(olc::vf2d{200, 200})
    {
        this->bullets = bullets;
    }
    
    bool moving = false;
    std::deque<Bullet*>* bullets;

    std::vector<olc::vf2d> getDrawingPoints()
    {
        olc::vf2d point1 = {std::sin(this->angle)*size+this->pos.x, std::cos(this->angle)*size+this->pos.y};
        olc::vf2d point2 = {std::sin(this->angle+(2*PI)/3)*size/2+this->pos.x, std::cos(this->angle+(2*PI)/3)*size/2+this->pos.y};
        olc::vf2d point3 = {std::sin(this->angle+(4*PI)/3)*size/2+this->pos.x, std::cos(this->angle+(4*PI)/3)*size/2+this->pos.y};

        std::vector<olc::vf2d> points = {point1, point2, point3};

        return points;
    }

    void spawnBullet()
    {
        bullets->push_back(new Bullet(this->pos, this->angle));
    }

    std::pair<float, Asteroid*> getClosestAsteroid(std::vector<Asteroid*>* asteroids)
    {
        std::vector<std::pair<float, Asteroid*>> distances;

        for (Asteroid* asteroid : *asteroids)
        {
            distances.push_back(std::make_pair((this->distanceToObject(*asteroid) - asteroid->getSize()), asteroid));
        }


        std::sort(distances.begin(), distances.end());
        return distances[0];
    }
};

class AsteroidsGame : public olc::PixelGameEngine
{
public:
	AsteroidsGame()
	{
		sAppName = "Asteroids";
	}

public:
std::deque<Bullet*> bullets;
Ship ship = Ship(&bullets);
std::vector<Asteroid*> asteroids;

	bool OnUserCreate() override
	{
        asteroids.push_back(new Asteroid({50, 50}, std::rand()%360, 3, 40));
        return true;
	}

	bool OnUserUpdate(float fElapsedTime) override
	{
        Clear(olc::BLACK);
        updateShip(fElapsedTime);
        updateBullets();
        updateAsteroids();
        drawBullets();
        drawShip();
        drawAsteroids();
		return true;
	}

    void updateShip(float elapsedTime)
    {
        if(GetKey(olc::A).bHeld) ship.rotateBy(elapsedTime/1*3);
        if(GetKey(olc::D).bHeld) ship.rotateBy(elapsedTime/-1*3);
        if(GetKey(olc::W).bHeld)
            ship.moving = true;
        else
            ship.moving = false;

        if(ship.moving)
        {
            ship.addSpeed(0.1);
        } else {
            ship.addSpeed(-0.1);
        }

        ship.moveBySpeed();
        if(GetKey(olc::SPACE).bPressed) ship.spawnBullet();
    }

    void drawShip()
    {
        std::vector<olc::vf2d> shipDrawPoints = ship.getDrawingPoints();
        DrawTriangle(shipDrawPoints[0], shipDrawPoints[1], shipDrawPoints[2]);
        if(ship.moving)
        {
            olc::vf2d enginePos = {std::sin(ship.getAngle()+PI)*4+ship.getPos().x, std::cos(ship.getAngle()+PI)*4+ship.getPos().y};
            DrawCircle(enginePos, 2);
        }
        if (ship.getClosestAsteroid(&asteroids).first < 0)
        {
            FillRect({0, 0}, {10, 10}, olc::RED);
        }
    }

    void drawBullets()
    {
        for (auto bullet : bullets)
        {
            std::vector<olc::vf2d> points = bullet->getDrawingPoints();
            DrawLine(points[0], points[1]);
        }
    }

    void updateBullets()
    {
        for (Bullet* &bullet : bullets) 
        {
            bullet->moveBySpeed();
            bullet->lifetime -= 1;
        }

        if (bullets.size() > 0 && bullets[0]->lifetime <= 0)
        {
            bullets.pop_front();
        }
    }

    void updateAsteroids()
    {
        std::vector<Asteroid*> tempAsteroids = asteroids;
        std::vector<Asteroid*> newAsteroids;
        for (Asteroid* asteroid : tempAsteroids)
        {
            asteroid->moveBySpeed();
            if (bullets.size() > 0)
            {
                std::pair<float, Bullet*> closestBullet = asteroid->getClosestBullet(&bullets);
                if (closestBullet.first < 0)
                {
                    bullets.erase(std::find(bullets.begin(), bullets.end(), closestBullet.second));
                    if (asteroid->getSize() > 10)
                    {
                        newAsteroids.push_back(new Asteroid(asteroid->getPos(), std::rand()%360, 3, asteroid->getSize()/2));
                        newAsteroids.push_back(new Asteroid(asteroid->getPos(), std::rand()%360, 3, asteroid->getSize()/2));
                    }
                    asteroids.erase(std::find(asteroids.begin(), asteroids.end(), asteroid));
                }
            }
        }

        for (Asteroid* newAsteroid : newAsteroids)
        {
            asteroids.push_back(newAsteroid);
        }

        if (asteroids.size() <= 3)
        {
            asteroids.push_back(new Asteroid({std::rand()%ScreenWidth(), std::rand()%ScreenHeight()}, std::rand()%360, 3, 40));
        }
    }

    void drawAsteroids()
    {
        for (Asteroid* asteroid : asteroids)
        {
            DrawCircle(asteroid->getPos(), asteroid->getSize());
        }

        //DrawString({0, 0}, std::to_string(asteroids[0]->distanceToBullet(bullets[0])));
    }
};

int main()
{
	AsteroidsGame game;
	if (game.Construct(HEIGHT, WIDTH, 1, 1))
		game.Start();
	return 0;
}
