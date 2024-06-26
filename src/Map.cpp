#include <ncurses.h>
#include <cstdlib>
#include <ctime>
#include <random>
#include <unistd.h>
#include <vector>
#include <chrono>
#include <utility>

const int HEIGHT = 21;
const int WIDTH = 21;
int map[HEIGHT][WIDTH];

struct Position
{
    int x, y;
};
struct Gate
{
    Position pos1;
    Position pos2;
};
std::pair<Position, Position> gates;

class Snake
{
private:
    std::vector<Position> body;
    int direction;
    int prevDirection;

public:
    Snake(int mapWidth, int mapHeight)
    {
        direction = 2;
        prevDirection = 2;

        int startX = mapWidth / 2;
        int startY = mapHeight / 2;

        body.push_back({startX, startY});
        body.push_back({startX - 1, startY});
        body.push_back({startX - 2, startY});

        map[startY][startX] = 3;
        map[startY][startX - 1] = 4;
        map[startY][startX - 2] = 4;
    }

    void changeDirection(int newDirection)
    {
        if (newDirection != prevDirection && abs(newDirection - prevDirection) != 2)
        {
            direction = newDirection;
        }
    }

    void move()
    {
        Position newHead = body[0];

        switch (direction)
        {
        case 1:
            newHead.y--;
            break;
        case 2:
            newHead.x++;
            break;
        case 3:
            newHead.y++;
            break;
        case 4:
            newHead.x--;
            break;
        }
        if (map[newHead.y][newHead.x] == 7 || map[newHead.y][newHead.x] == 8)
        {
            if (map[newHead.y][newHead.x] == 7)
            {
                newHead = gates.second;
            }
            else
            {
                newHead = gates.first;
            }

            // Adjust new head position if gate is on the edge
            if (newHead.x <= 0)
                newHead.x = 1;
            if (newHead.x >= WIDTH - 1)
                newHead.x = WIDTH - 2;
            if (newHead.y <= 0)
                newHead.y = 1;
            if (newHead.y >= HEIGHT - 1)
                newHead.y = HEIGHT - 2;
            if (direction == 1 && map[newHead.y - 1][newHead.x] == 1)
            {
                direction = 3;
            }
            else if (direction == 2 && map[newHead.y - 1][newHead.x] == 1)
            {
                direction = 4;
            }
            else if (direction == 3 && map[newHead.y - 1][newHead.x] == 1)
            {
                direction = 1;
            }
            else if (direction == 4 && map[newHead.y - 1][newHead.x] == 1)
            {
                direction = 2;
            }
            map[gates.first.y][gates.first.x] = 0;
            map[gates.second.y][gates.second.x] = 0;
        }

        if (checkCollision(newHead))
        {
            endwin();
            printf("Game Over! Score: %ld\n", body.size());
            exit(0);
        }

        bool grew = false;
        if (map[newHead.y][newHead.x] == 5)
        {
            grew = true;
            map[newHead.y][newHead.x] = 0;
        }
        else if (map[newHead.y][newHead.x] == 6)
        {
            if (body.size() <= 3)
            {
                endwin();
                printf("Game Over! Score: %ld\n", body.size());
                exit(0);
            }
            Position tail = body.back();
            body.pop_back();
            map[tail.y][tail.x] = 0;
            map[newHead.y][newHead.x] = 0;
        }

        body.insert(body.begin(), newHead);
        map[newHead.y][newHead.x] = 3;

        if (!grew)
        {
            Position tail = body.back();
            body.pop_back();
            map[tail.y][tail.x] = 0;
            if (body.size() < 3)
            {
                endwin();
                printf("Game Over! Score: %ld\n", body.size());
                exit(0);
            }
        }

        prevDirection = direction;
    }

    void draw()
    {
        for (size_t i = 0; i < body.size(); ++i)
        {
            if (i == 0)
            {
                mvaddch(body[i].y, body[i].x, 'H');
            }
            else
            {
                mvaddch(body[i].y, body[i].x, 'o');
            }
        }
        mvaddch(gates.first.y, gates.first.x, 'G');
        mvaddch(gates.second.y, gates.second.x, 'G');
    }

    bool checkCollision(Position newHead)
    {
        if (newHead.x <= 0 || newHead.x >= WIDTH - 1 || newHead.y <= 0 || newHead.y >= HEIGHT - 1)
        {
            return true;
        }

        for (size_t i = 0; i < body.size(); i++)
        {
            if (newHead.x == body[i].x && newHead.y == body[i].y)
            {
                return true;
            }
        }

        return false;
    }

    void clearOldItems()
    {
        for (int i = 1; i < HEIGHT - 1; i++)
        {
            for (int j = 1; j < WIDTH - 1; j++)
            {
                if (map[i][j] == 5 || map[i][j] == 6)
                {
                    map[i][j] = 0;
                }
                else if (map[i][j] == 7 || map[i][j] == 8)
                {
                    map[gates.first.y][gates.first.x] = 0;
                    map[gates.second.y][gates.second.x] = 0;
                }
            }
        }
    }

    void placeItems()
    {
        clearOldItems();

        std::random_device rd;
        std::mt19937 gen(rd());
        std::uniform_int_distribution<int> dis(1, WIDTH - 2);

        int growthItemsPlaced = 0;
        int poisonItemsPlaced = 0;

        while (growthItemsPlaced < 3 || poisonItemsPlaced < 2)
        {
            int x = dis(gen);
            int y = dis(gen);
            if (map[y][x] == 0)
            {
                if (growthItemsPlaced < 3)
                {
                    map[y][x] = 5;
                    growthItemsPlaced++;
                }
                else if (poisonItemsPlaced < 2)
                {
                    map[y][x] = 6;
                    poisonItemsPlaced++;
                }
            }
        }
        placeGates();
    }
    void placeGates()
    {
        std::random_device rd;
        std::mt19937 gen(rd());
        std::uniform_int_distribution<int> dis(1, WIDTH - 2);

        Position gate1, gate2;
        do
        {
            gate1 = {dis(gen), dis(gen)};
        } while (map[gate1.y][gate1.x] != 0);

        do
        {
            gate2 = {dis(gen), dis(gen)};
        } while (map[gate2.y][gate2.x] != 0 || (gate1.x == gate2.x && gate1.y == gate2.y));

        gates = {gate1, gate2};

        map[gate1.y][gate1.x] = 7; // Gate 1
        map[gate2.y][gate2.x] = 8; // Gate 2
    }

    void handleItems()
    {
        static auto lastItemTime = std::chrono::high_resolution_clock::now();
        auto currentTime = std::chrono::high_resolution_clock::now();
        std::chrono::duration<float> elapsed = currentTime - lastItemTime;

        if (elapsed.count() >= 5.0)
        {
            placeItems();
            lastItemTime = currentTime;
        }
    }
};

void initMap()
{
    for (int i = 0; i < HEIGHT; i++)
    {
        for (int j = 0; j < WIDTH; j++)
        {
            if (i == 0 || i == HEIGHT - 1 || j == 0 || j == WIDTH - 1)
            {
                map[i][j] = 1;
            }
            else
            {
                map[i][j] = 0;
            }
        }
    }
}

void printMap()
{
    for (int i = 0; i < HEIGHT; i++)
    {
        for (int j = 0; j < WIDTH; j++)
        {
            char displayChar = ' ';
            switch (map[i][j])
            {
            case 1:
                displayChar = '#';
                break;
            case 3:
                displayChar = 'H';
                break;
            case 4:
                displayChar = 'o';
                break;
            case 5:
                displayChar = '+';
                break;
            case 6:
                displayChar = '-';
                break;
            case 7:
                displayChar = 'G';
                break; // Gate1
            case 8:
                displayChar = 'G';
                break; // Gate2
            }
            mvaddch(i, j, displayChar);
        }
    }
}

void gameLoop()
{
    Snake snake(WIDTH, HEIGHT);
    snake.placeItems();

    bool gameStarted = false;

    int ch;
    auto lastUpdateTime = std::chrono::high_resolution_clock::now();
    timeout(100);

    mvprintw(HEIGHT + 1, 0, "Press any arrow to start the game");
    refresh();

    while ((ch = getch()) != 'q')
    {
        if (!gameStarted && ch != ERR)
        {
            switch (ch)
            {
            case KEY_UP:
                snake.changeDirection(1);
                gameStarted = true;
                break;
            case KEY_RIGHT:
                snake.changeDirection(2);
                gameStarted = true;
                break;
            case KEY_DOWN:
                snake.changeDirection(3);
                gameStarted = true;
                break;
            case KEY_LEFT:
                snake.changeDirection(4);
                gameStarted = true;
                break;
            }
            clear();
        }

        if (gameStarted)
        {
            if (ch != ERR)
            {
                switch (ch)
                {
                case KEY_UP:
                    snake.changeDirection(1);
                    break;
                case KEY_RIGHT:
                    snake.changeDirection(2);
                    break;
                case KEY_DOWN:
                    snake.changeDirection(3);
                    break;
                case KEY_LEFT:
                    snake.changeDirection(4);
                    break;
                }
            }

            auto currentTime = std::chrono::high_resolution_clock::now();
            std::chrono::duration<float> elapsed = currentTime - lastUpdateTime;
            if (elapsed.count() >= 0.1)
            { // Increased speed
                snake.move();
                snake.draw();
                snake.handleItems();
                printMap();
                refresh();
                lastUpdateTime = currentTime;
            }
        }
    }
}

int main()
{
    initscr();
    cbreak();
    keypad(stdscr, TRUE);
    noecho();
    curs_set(0);
    start_color();

    initMap();
    printMap();

    gameLoop();

    endwin();
    return 0;
}
