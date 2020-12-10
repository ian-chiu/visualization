#include <SFML/Graphics.hpp>
#include <Thor/Shapes/ConcaveShape.hpp>

#include "App.h"
#include "Solution.h"
#include "cmake_variables.h"
#include "gtl_poly_types.h"
using namespace gtl::operators;

int main()
{
    try
    {
        Solution solution(CMAKE_SOURCE_DIR + "/data/input.txt", CMAKE_SOURCE_DIR + "/data/output.txt");
        solution.read_operations();
        
        // App app{1920, 1080};
        App app{1280, 720};
        // app.set_operations(solution);

        while (app.isWindowOpen())
        {
            // ----------------EXECUTE OPERATIONS----------------
            if (!app.isAllDone)
                solution.execute_and_render_operations(app);

            // ------------ALL OPERATIONS ARE DONE-----------------
            app.render(solution);
        }
    }
    catch(const std::exception& e)
    {
        std::cerr << e.what() << '\n';
        ImGui::SFML::Shutdown();
        return -1;
    }

    ImGui::SFML::Shutdown();
    return 0;
}