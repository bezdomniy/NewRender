#include <fmt/core.h>

class Renderer
{
public:
    Renderer(std::string name);

    void run()
    {
        fmt::print("Hello {}!\n", this->name);
    }

private:
    std::string name;
};