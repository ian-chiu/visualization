#include "App.h"
#include <stdexcept>

// contructor
App::App(int w, int h)
   : win_width(w), win_height(h)
{
    if (!texture.create(500, 500))
    {
        std::cerr << "Cannot create texture!";
        std::exit(-1);
    }
    sf::Image image;
    image.create(500, 500, sf::Color::White);
    texture.update(image);
    texture.setRepeated(true);

    window.create(sf::VideoMode(win_width, win_height), "Visualization", sf::Style::Close | sf::Style::Resize);
    window.setVerticalSyncEnabled(true);
    ImGui::SFML::Init(window, false);
    window.resetGLStates();

    std::string font_path = CMAKE_SOURCE_DIR + "/resource/segoeui.ttf";
    ImGui::GetIO().Fonts->Clear(); 
    ImGui::GetIO().Fonts->AddFontFromFileTTF(font_path.c_str(), 20.f);
    ImGui::GetIO().Fonts->AddFontFromFileTTF(font_path.c_str(), 25.f);
    ImGui::GetIO().Fonts->AddFontDefault();
    ImGui::SFML::UpdateFontTexture(); 

    ImGuiStyle& style = ImGui::GetStyle();
    style.WindowRounding    =   0.0f;
    style.WindowMinSize     =   ImVec2(1.0f, 1.0f);
    style.Colors[ImGuiCol_WindowBg]              =   ImVec4(20.0f / 255.0f, 20.0f / 255.0f, 20.0f / 255.0f, 0.8f);
    style.Colors[ImGuiCol_Border]                =   ImVec4(1.0f, 1.0f, 1.0f, 0.0f);
    style.Colors[ImGuiCol_Button]                =   ImVec4(1.0f, 1.0f, 1.0f, 0.0f);
    style.Colors[ImGuiCol_ButtonHovered]         =   ImVec4(100.0f / 255.0f, 100.0f / 255.0f, 100.0f / 255.0f, 1.0f);
    style.Colors[ImGuiCol_ButtonActive]          =   ImVec4(10.0f / 255.0f, 10.0f / 255.0f, 10.0f / 255.0f, 1.0f);
    style.Colors[ImGuiCol_ResizeGrip]            =   ImVec4(50.0f / 255.0f, 50.0f / 255.0f, 50.0f / 255.0f, 1.0f);
    style.Colors[ImGuiCol_ResizeGripHovered]     =   ImVec4(100.0f / 255.0f, 100.0f / 255.0f, 100.0f / 255.0f, 1.0f);
    style.Colors[ImGuiCol_ResizeGripActive]      =   ImVec4(30.0f / 255.0f, 30.0f / 255.0f, 30.0f / 255.0f, 1.0f);
    style.Colors[ImGuiCol_FrameBg]               =   ImVec4(25.0f / 255.0f, 25.0f / 255.0f, 25.0f / 255.0f, 0.8f);
    style.Colors[ImGuiCol_TitleBgActive]         =   ImVec4(36.0f / 255.0f, 36.0f / 255.0f, 36.0f / 255.0f, 0.8f);

    memset(InputBuf, 0, sizeof(InputBuf));

    camera.reset(sf::FloatRect(0.0f, 0.0f, win_width, win_height));
    window.setView(camera);
    lines.reserve(100000);
}

void App::render(const Solution &sol, bool can_draw_shapes)
{
    sf::Event event;
    sf::Time elapsed_time = deltaClock.getElapsedTime();
    int ms = elapsed_time.asMilliseconds();

    while (window.pollEvent(event))
    {
        ImGui::SFML::ProcessEvent(event);
        if (event.type == sf::Event::Closed)
        {
            window.close();
            std::exit(0);
        }
        else if (event.type == sf::Event::Resized)
        {
            // update the view to the new size of the window
            win_height = event.size.height;
            win_width = event.size.width;
            camera.setSize(win_width * worldScale, win_height * worldScale);
            camera.setCenter(focusPoint);
            window.setView(camera);
        }
        else if (event.type == sf::Event::MouseWheelMoved)
        {
            if (event.mouseWheel.delta > 0)
            {
                worldScale /= 1.05f;
                camera.zoom(1.0f / 1.05f);
            }
            else
            {
                worldScale *= 1.05f;
                camera.zoom(1.05f);
            }
            window.setView(camera);
        }
        else if (event.type == sf::Event::KeyPressed)
        {
            switch (event.key.code) 
            {
            case sf::Keyboard::Num1:
                if (isAllDone)  split_mode = true;
                break;
            
            case sf::Keyboard::Num2:
                if (isAllDone)  split_mode = false;
                break;
            
            case sf::Keyboard::Enter:
                if (!isAllDone && !can_show_inputWindow) 
                    can_start_step = true;
                break;

            case sf::Keyboard::O:
                focusMode = !focusMode;
                break;

            case sf::Keyboard::F:
                camera.setCenter(focusPoint);
                window.setView(camera);
                break;

            case sf::Keyboard::I:
                if (step_cnt == 0) 
                    can_show_inputWindow = true;
                break;

            case sf::Keyboard::Space:
                isPause = true;
                break;

            case sf::Keyboard::Escape:
                can_show_inputWindow = false;
                break;
            }   
        }
        // else if (event.type == sf::Event::MouseButtonPressed)
        // {
        //     if (event.mouseButton.button == sf::Mouse::Left && !ImGui::IsAnyWindowFocused())
        //     {
        //         sf::Vector2i pixelPos = sf::Mouse::getPosition(window);
        //         sf::Vector2f worldPos = window.mapPixelToCoords(pixelPos);
        //         sf::Vector2f mousePlotPos = plotPos(worldPos);
        //         camera.setCenter(mousePlotPos);
        //         window.setView(camera);
        //     }
        // }
    }

    if (!can_show_inputWindow)
    {
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::Up) || sf::Keyboard::isKeyPressed(sf::Keyboard::W))
        {
            camera.move( 0.0f, -camera_speed * ms * worldScale );
            window.setView(camera);
        }
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::Down) || sf::Keyboard::isKeyPressed(sf::Keyboard::S))
        {
            camera.move( 0.0f, camera_speed * ms * worldScale );
            window.setView(camera);
        }
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::Left) || sf::Keyboard::isKeyPressed(sf::Keyboard::A))
        {
            camera.move( -camera_speed * ms * worldScale, 0.0f );
            window.setView(camera);
        }
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::Right) || sf::Keyboard::isKeyPressed(sf::Keyboard::D))
        {
            camera.move( camera_speed * ms * worldScale, 0.0f );
            window.setView(camera);
        }
        if (worldScale >= 0.01f && sf::Keyboard::isKeyPressed(sf::Keyboard::Add))
        {
            worldScale /= 1.05f;
            camera.zoom(1.0f / 1.05f);
            window.setView(camera);
        }
        if (worldScale <= 10000000.0f &&sf::Keyboard::isKeyPressed(sf::Keyboard::Subtract))
        {
            worldScale *= 1.05f;
            camera.zoom(1.05f);
            window.setView(camera);
        }
    }

    ImGui::SFML::Update(window, deltaClock.restart());

    ImGui::PushFont(ImGui::GetIO().Fonts->Fonts[2]);

    showMemuBar();
    if (can_show_hintBar)           showHintBar();
    if (can_show_colorSelector)     showColorSelector();
    if (can_show_inputWindow)       showInputWindow(sol);
    showBottomBar(sol);

    ImGui::PopFont();

    window.clear(bgColor);
    if (can_draw_shapes)
    {
        // split_mode ? draw_rectangles(sol.output_rects) : draw_polygon_set(sol.polygon_set);
        draw_polygon_set(sol.polygon_set);
        if (split_mode)
            draw_rects_edge(sol.output_rects);
    }

    ImGui::SFML::Render(window);
    window.display();
}

bool App::isWindowOpen()
{
    return window.isOpen();
}

// ----------------private methods------------------
sf::Vector2f App::plotPos(float x, float y)
{
    return {x, win_height - y};
}

sf::Vector2f App::plotPos(const sf::Vector2f &pt)
{
    return {pt.x, win_height - pt.y};
}

sf::Vector2i App::plotPos(const sf::Vector2i &pt)
{
    return {pt.x, win_height - pt.y};
}

void App::draw_rectangles(const std::vector<Rect> &rects, const sf::Color &color)
{
    sf::RectangleShape rectShape;
    rectShape.setTexture(&texture);
    rectShape.setFillColor(color);

    for (auto rect : rects)
    {
        float rect_width = rect.get(gtl::HORIZONTAL).high() - rect.get(gtl::HORIZONTAL).low();
        float rect_height = rect.get(gtl::VERTICAL).high() - rect.get(gtl::VERTICAL).low();
        rectShape.setSize(sf::Vector2f(rect_width, rect_height));
        rectShape.setPosition(plotPos(gtl::xl(rect), gtl::yh(rect)));
        window.draw(rectShape);
    }
}

void App::draw_rects_edge(const std::vector<Rect> &rects)
{
    for (auto rect : rects)
    {
        sf::Vertex lb(plotPos(gtl::xl(rect), gtl::yl(rect)), sf::Color::White);
        sf::Vertex rt(plotPos(gtl::xh(rect), gtl::yh(rect)), sf::Color::White);
        sf::Vertex lt(plotPos(gtl::xl(rect), gtl::yh(rect)), sf::Color::White);
        sf::Vertex rb(plotPos(gtl::xh(rect), gtl::yl(rect)), sf::Color::White);

        lines.emplace_back(lb);
        lines.emplace_back(rb);

        lines.emplace_back(rb);
        lines.emplace_back(rt);

        lines.emplace_back(rt);
        lines.emplace_back(lt);

        lines.emplace_back(lt);
        lines.emplace_back(lb);
    }
    window.draw(&lines[0], lines.size(), sf::Lines);
    lines.clear();
}

void App::draw_polygon_set(const PolygonSet &ps)
{
    static std::vector<Rect> rect_shapes;
    static sf::Color shape_color;
    int polygon_cnt = 0;
    for (const auto &poly : ps)
    {
        shape_color = boardColor;
        if (!isAllDone && polygon_cnt == ps.size() - 1 )
            shape_color = operColor;
        
        gtl::get_rectangles(rect_shapes, poly);
        draw_rectangles(rect_shapes, shape_color);
        rect_shapes.clear();

        polygon_cnt++;
    }
}

void App::showMemuBar()
{
    
    if (ImGui::BeginMainMenuBar())
    {
        memuBarHeight = ImGui::GetWindowHeight();
        if (ImGui::BeginMenu("File"))
        {
            ImGui::EndMenu();
        }
        if (ImGui::BeginMenu("Tools"))
        {
            if (ImGui::MenuItem("Hint")) { can_show_hintBar = true; }
            if (ImGui::MenuItem("Color Selector")) { can_show_colorSelector =  true; } 
            ImGui::EndMenu();
        }
        ImGui::EndMainMenuBar();
    }
}

void App::showBottomBar(const Solution &sol)
{
    static sf::Vector2f last_mouse_pos, last_camera_center;
    static float last_world_scale;

    ImGuiWindowFlags window_flags = 
        ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoScrollbar | 
        ImGuiWindowFlags_NoScrollWithMouse | ImGuiWindowFlags_NoCollapse | 
        ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoBringToFrontOnFocus | ImGuiWindowFlags_NoTitleBar;

    ImGui::SetNextWindowPos(ImVec2(0.0f, ImGui::GetIO().DisplaySize.y), 0, ImVec2(0.0f, 1.0f));

    ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(5.0f, 1.0f));
    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(10.0f, 3.0f));
    ImGui::PushStyleColor(ImGuiCol_WindowBg, ImVec4(36.0f / 255.0f, 36.0f / 255.0f, 36.0f / 255.0f, 1.0f));
    if (step_cnt > 0)  ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(80.0f / 255.0f, 80.0f / 255.0f, 80.0f / 255.0f, 1.0f));

    if (ImGui::Begin("test", (bool *)0, window_flags))
    {
        ImGui::SetWindowSize(ImVec2(ImGui::GetIO().DisplaySize.x, 20.0f));

        if (step_cnt <= 0)
        {
            sf::Vector2i pixelPos = sf::Mouse::getPosition(window);
            sf::Vector2f worldPos = window.mapPixelToCoords(pixelPos);
            sf::Vector2f mousePlotPos = plotPos(worldPos.x, worldPos.y);
            last_mouse_pos = mousePlotPos;
            ImGui::Text("Mouse Position: (%.1f,%.1f)\t", mousePlotPos.x, mousePlotPos.y);
            ImGui::SameLine();

            last_world_scale = worldScale * 100.0f;
            ImGui::Text("World Scale: %.1f %%\t", worldScale * 100.0f);
            ImGui::SameLine();

            last_camera_center = camera.getCenter();
            ImGui::Text("Camera Center: (%.1f,%.1f)\t", last_camera_center.x, win_height - last_camera_center.y);
            ImGui::SameLine();

            ImGui::Text("Operation Order: ");
            int oper_cnt = 0;
            for (const auto &oper_str : sol.operations) 
            {
                if (sol.order_idx == oper_cnt)
                {
                    ImGui::SameLine();
                    ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1.0f, 0.4f, 0.4f, 1.0f));
                    ImGui::Text((oper_str + " ").c_str());
                    ImGui::PopStyleColor();
                    ImGui::SameLine();
                }
                else
                {
                    ImGui::SameLine();
                    ImGui::Text((oper_str + " ").c_str());
                }
                oper_cnt++;
            }
        }
        else
        {
            ImGui::Text("Mouse Position: (%.1f,%.1f)\t", last_mouse_pos.x, last_mouse_pos.y);
            ImGui::SameLine();

            ImGui::Text("World Scale: %.1f %%\t", last_world_scale * 100.0f);
            ImGui::SameLine();

            ImGui::Text("Camera Center: (%.1f,%.1f)\t", last_camera_center.x, win_height - last_camera_center.y);
            ImGui::SameLine();

            ImGui::Text("Operation Order: ");
            for (const auto &oper_str : sol.operations) 
            {
                ImGui::SameLine();
                ImGui::Text((oper_str + " ").c_str());
            }
        }
        ImGui::End();
    }   
    ImGui::PopStyleVar();
    ImGui::PopStyleVar();
    ImGui::PopStyleColor();
    if (step_cnt > 0)  
        ImGui::PopStyleColor();
}

void App::showHintBar()
{
    ImGuiWindowFlags window_flags = 
        ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoScrollbar | 
        ImGuiWindowFlags_NoScrollWithMouse | ImGuiWindowFlags_NoCollapse | 
        ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoBringToFrontOnFocus | ImGuiWindowFlags_NoTitleBar;

    ImGui::SetNextWindowPos(ImVec2(0.0f, memuBarHeight), 0, ImVec2(0.0f, 0.0f));

    if (ImGui::Begin("Hint", &can_show_hintBar, window_flags))
    {
        ImGui::SetWindowSize(ImVec2(ImGui::GetIO().DisplaySize.x, 30.0f));
        ImGui::Text(hint_text.c_str());
        ImGui::SameLine(ImGui::GetWindowWidth() - 25.0f);
        if (ImGui::Button("X"))
            can_show_hintBar = false;

        ImGui::End();
    }  
}

void App::showColorSelector()
{
    ImGuiWindowFlags window_flags = ImGuiWindowFlags_AlwaysAutoResize;
    if (ImGui::Begin("Color Selector", &can_show_colorSelector))
    {
        if (ImGui::ColorEdit3("Background color", bg_rbg)) 
        {
            bgColor.r = static_cast<sf::Uint8>(bg_rbg[0] * 255.0f);
            bgColor.g = static_cast<sf::Uint8>(bg_rbg[1] * 255.0f);
            bgColor.b = static_cast<sf::Uint8>(bg_rbg[2] * 255.0f);
        }
        if (ImGui::ColorEdit3("Board color", board_rbg)) 
        {
            boardColor.r = static_cast<sf::Uint8>(board_rbg[0] * 255.0f);
            boardColor.g = static_cast<sf::Uint8>(board_rbg[1] * 255.0f);
            boardColor.b = static_cast<sf::Uint8>(board_rbg[2] * 255.0f);
        }
        if (!isAllDone)
        {
            if (ImGui::ColorEdit3("Operation color", oper_rbg)) 
            {
                operColor.r = static_cast<sf::Uint8>(oper_rbg[0] * 255.0f);
                operColor.g = static_cast<sf::Uint8>(oper_rbg[1] * 255.0f);
                operColor.b = static_cast<sf::Uint8>(oper_rbg[2] * 255.0f);
            }
        }
        ImGui::End();
    }
    
}

void App::showInputWindow(const Solution &sol)
{
    ImGuiWindowFlags window_flags = ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_AlwaysAutoResize;
    ImGui::PushStyleColor(ImGuiCol_WindowBg, ImVec4(36.0f / 255.0f, 36.0f / 255.0f, 36.0f / 255.0f, 0.8f));

    if (ImGui::Begin(" ", nullptr, window_flags))
    {
        ImGui::SetWindowPos(ImVec2(ImGui::GetIO().DisplaySize.x / 2.0f - ImGui::GetWindowWidth() / 2.0f, 80.0f));

        std::string message = sol.curr_oper + " remaining polygons: " + std::to_string(sol.nRemains);
        message += ". Please enter how many steps you want to operate.";
        ImGui::Text(message.c_str());
        ImGui::Separator();

        ImGuiInputTextFlags input_text_flags = ImGuiInputTextFlags_EnterReturnsTrue;
        if (ImGui::InputText("Input", InputBuf, IM_ARRAYSIZE(InputBuf), input_text_flags))
        {
            char* s = InputBuf;
            Strtrim(s);
            if (s[0]) 
            {
                ExecCommand(s);
                can_show_inputWindow = false;
            }
                
            strcpy(s, "");
        }

        // Auto-focus on window apparition
        ImGui::SetItemDefaultFocus();
        ImGui::SetKeyboardFocusHere(-1); // Auto focus previous widget

        if (!ImGui::IsWindowFocused())
            can_show_inputWindow = false;
        
        ImGui::End();
    }
    ImGui::PopStyleColor();
}

void App::ExecCommand(const char* command_line)
{
    if (Stricmp(command_line, "skip") == 0) 
    {
        can_start_step = true;
    }
    else
    {
        step_cnt = atoi(command_line);
        if (step_cnt > 0)
            can_start_step = true;
        else 
            step_cnt = 0;
    }
}