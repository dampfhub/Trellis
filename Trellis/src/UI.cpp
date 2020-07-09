#include "ui.h"

#include "client_server.h"
#include "glfw_handler.h"
#include "imgui_helpers.h"
#include "network_manager.h"
#include "gui.h"

#include <iostream>
#include <iterator>
#include <ctime>
#include <regex>
#include <queue>
#include <stack>
#include <random>

using Data::ClientInfo, Data::ChatMessage, Data::NetworkData, std::regex, std::regex_match,
    std::smatch, std::string, std::stringstream, std::queue, std::stack, std::random_device,
    std::mt19937, std::to_string, std::runtime_error, std::exception, std::out_of_range;

using nlohmann::json;

using namespace ImGui;

static string
to_lower(const string &str) {
    string ret = str;
    std::for_each(ret.begin(), ret.end(), [](char &c) { c = tolower(c); });
    return ret;
}

UI::~UI() {
    delete FileDialog;
}

UI::UI() {
    FileDialog       = new FileBrowser(ImGuiFileBrowserFlags_CloseOnEsc);
    ClientServer &cs = ClientServer::GetInstance();
    cs.ChannelSubscribe("CHAT_MSG", [this](NetworkData &&d) { handle_chat_msg(std::move(d)); });
    cs.ChannelSubscribe("JOIN", [this](NetworkData &&d) { handle_client_join(std::move(d)); });

    NetworkManager &    nm        = NetworkManager::GetInstance();
    std::vector<string> api_calls = {"Spells", "Monsters"};
    for (auto &s : api_calls) {
        string res;
        nm.HttpGetRequest("www.dnd5eapi.co", "/api/" + to_lower(s));
        // Spin until we get results. TODO don't do this
        while (!nm.HttpGetResults(res)) {}
        cached_results[s] = json::parse(res);
    }
}

void
UI::Draw(Page::page_list_t &pages, Page::page_list_it_t &active_page) {
    // Display file dialog if it's open
    FileDialog->Display();
    draw_main_node();
    draw_menu(pages, active_page);
    draw_page_select(pages, active_page);
    draw_page_settings(active_page);
    draw_client_list();
    draw_chat();
    draw_http_window();
    ShowDemoWindow();
}

void
UI::draw_main_node() {
    static ImGuiDockNodeFlags dockspace_flags = ImGuiDockNodeFlags_PassthruCentralNode;
    ImGuiWindowFlags          window_flags = ImGuiWindowFlags_MenuBar | ImGuiWindowFlags_NoDocking;
    ImGuiViewport *           viewport     = GetMainViewport();
    SetNextWindowPos(viewport->GetWorkPos());
    SetNextWindowSize(viewport->GetWorkSize());
    SetNextWindowViewport(viewport->ID);
    PushStyleVar(ImGuiStyleVar_WindowRounding, 0.0f);
    PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f);
    window_flags |= ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoCollapse |
                    ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove;
    window_flags |= ImGuiWindowFlags_NoBringToFrontOnFocus | ImGuiWindowFlags_NoNavFocus;
    window_flags |= ImGuiWindowFlags_NoBackground;

    PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.0f, 0.0f));
    Begin("DockSpace Demo", nullptr, window_flags);
    PopStyleVar();
    PopStyleVar(2);

    // DockSpace
    ImGuiIO &io           = GetIO();
    ImGuiID  dockspace_id = GetID("MyDockSpace");
    DockSpace(dockspace_id, ImVec2(0.0f, 0.0f), dockspace_flags);

    if (BeginMenuBar()) {
        if (BeginMenu("Windows")) { ImGui::EndMenu(); }
        if (BeginMenu("Clients")) {
            draw_client_list();
            ImGui::EndMenu();
        }
        EndMenuBar();
    }

    End();
}

void
UI::draw_menu(Page::page_list_t &pages, Page::page_list_it_t &active_page) {
    (void)pages;
    auto  no_border = ImStyleResource(ImGuiStyleVar_FrameBorderSize, 0.0f);
    GLFW &glfw      = GLFW::GetInstance();
    main_menu_open  = Begin("Trellis", nullptr);
    SetWindowPos(ImVec2(0.0f, 0.0f), ImGuiCond_FirstUseEver);
    SetWindowSize(ImVec2(150.0f, 200.0f), ImGuiCond_Once);
    SetNextItemWidth(100.0f);
    if (Button("Add", {-1, 0})) { OpenPopup("add_menu"); }
    if (IsItemHovered() && GImGui->HoveredIdTimer > 0.5f) {
        BeginTooltip();
        TextUnformatted("Add something to the page");
        EndTooltip();
    }
    // Things that can be added
    if (BeginPopup("add_menu")) {
        Text("Add Options");
        Separator();
        if (Selectable("Add from File")) { FileDialog->Open(); }
        AddFromPreset = Selectable("Add from Preset");
        if (Selectable("Add Page##0")) { PageAddOpen = true; }
        EndPopup();
    }

    // Page name popup
    if (PageAddOpen) { OpenPopup("Add Page##1"); }
    if (BeginPopupModal("Add Page##1", NULL, ImGuiWindowFlags_AlwaysAutoResize)) {
        Text("Page Name: ");
        SameLine();
        if (InputText("##edit", &page_name_buf, ImGuiInputTextFlags_EnterReturnsTrue)) {
            CloseCurrentPopup();
            PageName      = page_name_buf;
            page_name_buf = "";
            AddPage       = true;
            PageAddOpen   = false;
        }
        Separator();
        if (Button("OK", ImVec2(120, 0))) {
            CloseCurrentPopup();
            PageName      = page_name_buf;
            page_name_buf = "";
            AddPage       = true;
            PageAddOpen   = false;
        }
        SameLine();
        if (Button("Cancel", ImVec2(120, 0))) {
            CloseCurrentPopup();
            PageAddOpen = false;
        }
        EndPopup();
    }
    if (Button("Page Select", {-1, 0})) { PageSelectOpen = !PageSelectOpen; }
    if (IsItemHovered() && GImGui->HoveredIdTimer > 0.5f) {
        BeginTooltip();
        TextUnformatted("Select a page to view");
        EndTooltip();
    }
    if (Button("Page Settings", {-1, 0})) {
        if (active_page != pages.end()) {
            PageSettingsOpen = !PageSettingsOpen;
            page_name_buf    = (**active_page).Name;
            auto pg_dims     = (*active_page)->getCellDims();
            PageX            = pg_dims.x;
            PageY            = pg_dims.y;
        }
    }
    if (IsItemHovered() && GImGui->HoveredIdTimer > 0.5f) {
        BeginTooltip();
        TextUnformatted("Change page settings");
        EndTooltip();
    }
    if (Button("Chat", {-1, 0})) { chat_open = !chat_open; }
    if (IsItemHovered() && GImGui->HoveredIdTimer > 0.5f) {
        BeginTooltip();
        TextUnformatted("Chat with other players");
        EndTooltip();
    }
    static NetworkManager &nm = NetworkManager::GetInstance();
    if (Button("Info", {-1, 0})) { http_window_open = !http_window_open; }
    if (IsItemHovered() && GImGui->HoveredIdTimer > 0.5f) {
        BeginTooltip();
        TextUnformatted("Get info for spells/monsters/other");
        EndTooltip();
    }
    if (Button("Multi Viewports", {-1, 0})) {}
    if (IsItemHovered() && GImGui->HoveredIdTimer > 0.5f) {
        BeginTooltip();
        TextUnformatted("Toggle Multi Viewports");
        EndTooltip();
    }
    End();
}

void
UI::draw_page_select(Page::page_list_t &pages, Page::page_list_it_t &active_page) {
    if (!PageSelectOpen) { return; }
    Begin(
        "Page Select",
        &PageSelectOpen,
        ImGuiWindowFlags_NoResize | ImGuiWindowFlags_AlwaysAutoResize);
    int i = 0;
    Text("Switch Page");
    SameLine();
    Text("Player View");
    for (auto &page : pages) {
        // Color the active page differently
        if (page == *active_page) { PushStyleColor(ImGuiCol_Button, ImVec4(0.5, 0.5, 0.5, 1)); }
        if (Button(page->Name.c_str(), ImVec2(GetWindowSize().x * 0.5f, 0.0f))) {
            ActivePage = page->Uid;
        }
        if (page == *active_page) { PopStyleColor(1); }
        SameLine();
        if (RadioButton(("##" + std::to_string(i)).c_str(), &PlayerPageView, i)) {
            if (ClientServer::Started()) {
                static ClientServer &cs = ClientServer::GetInstance();
                cs.ChannelPublish("PLAYER_VIEW", page->Uid, "");
            }
        }
        i++;
    }
    End();
}

void
UI::draw_page_settings(Page::page_list_it_t &active_page) {
    if (!PageSettingsOpen) { return; }
    Page &pg = **active_page;
    Begin(
        "Page Settings",
        &PageSettingsOpen,
        ImGuiWindowFlags_NoResize | ImGuiWindowFlags_AlwaysAutoResize);
    Text("Page Name: ");
    InputText("##name", &page_name_buf);

    Text("Board Dimensions: ");
    InputInt("X", &PageX);
    InputInt("Y", &PageY);
    if (Button("Done", ImVec2(120, 0.0f))) {
        PageSettingsOpen = false;
        pg.setCellDims(glm::ivec2(PageX, PageY));
        pg.Name      = page_name_buf;
        SettingsPage = true;
    }
    End();
}

void
UI::ClearFlags() {
    AddFromPreset = false;
    AddPage       = false;
    SettingsPage  = false;
    PageName      = "";
}

void
UI::draw_client_list() {
    static GLFW &glfw = GLFW::GetInstance();
    if (ClientServer::Started()) {
        static ClientServer &cs = ClientServer::GetInstance();
        if (main_menu_open) {
            if (cs.ClientCount() > 0) {
                for (auto &inf : cs.getConnectedClients()) {
                    MenuItem(inf.Name.c_str());
                }
            }
        }
    }
}

void
UI::draw_chat() {
    if (!chat_open) { return; }
    auto window_size = ImStyleResource(ImGuiStyleVar_WindowMinSize, ImVec2(300, 400));
    auto bg_col      = ImStyleResource(ImGuiCol_WindowBg, IM_COL32(30, 30, 30, 255));

    Begin("Chat", nullptr);
    SetNextWindowSize(ImVec2(300, 400), ImGuiCond_Once);
    {
        ImVec2 win_size = GetContentRegionAvail();
        win_size.y -= 20;
        BeginChild("##chat_text", ImVec2(win_size.x, win_size.y * 0.8f));
        {
            string last_sender = "";
            for (auto &m : chat_messages) {
                if (m.SenderName != last_sender) {
                    {
                        auto c = ImStyleResource(ImGuiCol_Text, IM_COL32(34, 139, 34, 255));
                        Separator();
                        last_sender          = m.SenderName;
                        std::tm *t           = std::localtime(&m.TimeStamp);
                        auto     hr_standard = t->tm_hour % 12;
                        TextWrapped(
                            "%s - %d:%02d %s",
                            m.SenderName.c_str(),
                            hr_standard == 0 ? 12 : hr_standard,
                            t->tm_min,
                            t->tm_hour >= 12 ? "PM" : "AM");
                        Dummy(ImVec2(0.0f, 3.0f));
                    }
                    TextWrapped("%s", m.Msg.c_str());
                } else {
                    TextWrapped("%s\n", m.Msg.c_str());
                }
            }
            if (scroll_to_bottom) {
                SetScrollHere(1.0f);
                scroll_to_bottom = false;
            }
        }
        EndChild();
        if (InputTextMultiline(
                "##send_msg",
                &send_msg_buf,
                ImVec2(win_size.x, win_size.y * 0.15f),
                ImGuiInputTextFlags_EnterReturnsTrue | ImGuiInputTextFlags_CtrlEnterForNewLine)) {
            SetKeyboardFocusHere(-1);
            if (!send_msg_buf.empty()) { send_msg(); }
        }
        if (Button("Send", ImVec2(win_size.x, 0)) && !send_msg_buf.empty()) { send_msg(); }
    }
    End();
}

struct Token {
    enum TOK_TYPE { DONE, ROLL, NUM, ADD, MUL, SUB, LPAREN, RPAREN, UMINUS };
    TOK_TYPE type;
    string   token;
    int      prec = 0;
};

bool
is_op(const Token &tok) {
    return tok.type == Token::ADD || tok.type == Token::SUB || tok.type == Token::MUL ||
           tok.type == Token::UMINUS;
}

bool
is_num(const Token &tok) {
    return tok.type == Token::NUM || tok.type == Token::ROLL;
}

Token
tokenize_string(string &str, const Token *prev = nullptr) {
    smatch matches;
    int    index = str.find_first_not_of(' ');
    if (index != string::npos) { str = str.substr(index); }
    if (str.length() == 0) { return Token{Token::DONE, ""}; }
    if (regex_search(str, matches, regex("^[1-9][0-9]*d[1-9][0-9]*"))) {
        Token tok{Token::ROLL, matches[0]};
        str = matches.suffix();
        return tok;
    }
    if (regex_search(str, matches, regex("^[0-9]+"))) {
        Token tok{Token::NUM, matches[0]};
        str = matches.suffix();
        return tok;
    }
    if (str[0] == '+') {
        Token tok{Token::ADD, "+", 2};
        str = str.substr(1);
        return tok;
    }
    if (str[0] == '-') {
        if (prev == nullptr || is_op(*prev) || prev->type == Token::LPAREN) {
            Token tok{Token::UMINUS, "-", 2};
            str = str.substr(1);
            return tok;
        }
        Token tok{Token::SUB, "-", 2};
        str = str.substr(1);
        return tok;
    }
    if (str[0] == '*') {
        Token tok{Token::MUL, "*", 3};
        str = str.substr(1);
        return tok;
    }
    if (str[0] == '(') {
        Token tok{Token::LPAREN, "("};
        str = str.substr(1);
        return tok;
    }
    if (str[0] == ')') {
        Token tok{Token::RPAREN, ")"};
        str = str.substr(1);
        return tok;
    }
    throw runtime_error("error: unrecognized string: " + str);
}

queue<Token>
parse_roll_string(string str) {
    // https://en.wikipedia.org/wiki/Shunting-yard_algorithm
    Token        tok;
    Token *      prev = nullptr;
    queue<Token> out_queue;
    stack<Token> op_stack;
    while ((tok = tokenize_string(str, prev)).type != Token::DONE) {
        if (is_op(tok)) {
            while (!op_stack.empty() && op_stack.top().prec >= tok.prec &&
                   op_stack.top().type != Token::LPAREN) {
                out_queue.push(op_stack.top());
                op_stack.pop();
            }
            op_stack.push(tok);
        } else if (is_num(tok)) {
            out_queue.push(tok);
        } else if (tok.type == Token::LPAREN) {
            op_stack.push(tok);
        } else if (tok.type == Token::RPAREN) {
            while (!op_stack.empty() && op_stack.top().type != Token::LPAREN) {
                out_queue.push(op_stack.top());
                op_stack.pop();
            }
            if (op_stack.empty()) { throw runtime_error("error: unmatched  ')'"); }
            op_stack.pop();
        } else {
            throw runtime_error("error: unrecognized \"" + tok.token + "\"");
        }
        prev = &tok;
    }
    while (!op_stack.empty()) {
        out_queue.push(op_stack.top());
        op_stack.pop();
    }
    return out_queue;
}

void
UI::send_msg() {
    scroll_to_bottom               = true;
    static ClientServer &cs        = ClientServer::GetInstance();
    auto                 cmd_regex = regex(R"(^\s*/roll )");
    smatch               matches;
    ChatMessage          m(cs.Name, send_msg_buf);
    chat_messages.push_back(m);
    if (regex_search(send_msg_buf, matches, cmd_regex)) {
        random_device rd;
        mt19937       gen(rd());
        string        substr = send_msg_buf.substr(matches[0].length());
        try {
            auto           out_queue = parse_roll_string(substr);
            stack<int64_t> eval;
            while (!out_queue.empty()) {
                Token tok = out_queue.front();
                out_queue.pop();
                if (tok.type == Token::ROLL) {
                    int     index = tok.token.find('d');
                    int64_t num   = stoll(tok.token.substr(0, index));
                    int64_t die   = stoll(tok.token.substr(index + 1));
                    int64_t val   = 0;
                    for (int i = 0; i < num; i++) { val += 1 + gen() % die; }
                    eval.push(val);
                    std::cout << tok.token << " = " << val << std::endl;
                } else if (tok.type == Token::NUM) {
                    int64_t val = stoi(tok.token);
                    eval.push(val);
                } else if (tok.type == Token::UMINUS) {
                    if (eval.empty()) {
                        throw runtime_error("error: unmatched '" + tok.token + "'");
                    }
                    int64_t b = eval.top();
                    eval.pop();
                    eval.push(-b);
                    std::cout << "- " << b << " = " << -b << std::endl;
                } else {
                    if (eval.size() < 2) {
                        throw runtime_error("error: unmatched '" + tok.token + "'");
                    }
                    int64_t b = eval.top();
                    eval.pop();
                    int64_t a = eval.top();
                    eval.pop();
                    if (tok.type == Token::ADD) {
                        eval.push(a + b);
                        std::cout << a << " + " << b << " = " << a + b << std::endl;
                    } else if (tok.type == Token::SUB) {
                        eval.push(a - b);
                        std::cout << a << " - " << b << " = " << a - b << std::endl;
                    } else if (tok.type == Token::MUL) {
                        eval.push(a * b);
                        std::cout << a << " * " << b << " = " << a * b << std::endl;
                    }
                }
            }
            assert(eval.size() == 1);
            string message = to_string(eval.top());
            m              = ChatMessage(cs.Name + " rolling", substr + " =\n" + message);
            chat_messages.push_back(m);
            send_msg_buf = "";
            cs.ChannelPublish("CHAT_MSG", m.Uid, m);
        } catch (runtime_error &e) {
            m = ChatMessage(cs.Name + " rolling", e.what());
            chat_messages.push_back(m);
            send_msg_buf = "";
        } catch ([[maybe_unused]] out_of_range &e) {
            m = ChatMessage(cs.Name + " rolling", "error: input out of range");
            chat_messages.push_back(m);
            send_msg_buf = "";
        }
        return;
    }
    send_msg_buf = "";
    cs.ChannelPublish("CHAT_MSG", m.Uid, m);
    // TODO Some sort of caching on the server side should probably happen here
}

void
UI::handle_chat_msg(Data::NetworkData &&q) {
    auto m = q.Parse<ChatMessage>();
    chat_messages.push_back(m);
    scroll_to_bottom = true;
}

void
UI::handle_client_join(NetworkData &&q) {
    static ClientServer &cs = ClientServer::GetInstance();
    for (auto &m : chat_messages) { cs.ChannelPublish("CHAT_MSG", m.Uid, m, q.Uid); }
}

void
UI::draw_http_window() {
    if (!http_window_open) { return; }
    static NetworkManager &nm = NetworkManager::GetInstance();
    auto window_size          = ImStyleResource(ImGuiStyleVar_WindowMinSize, ImVec2(300, 400));
    auto bg_col               = ImStyleResource(ImGuiCol_WindowBg, IM_COL32(30, 30, 30, 255));

    Begin("Info", nullptr);
    SetNextWindowSize(ImVec2(500, 500), ImGuiCond_Once);
    {
        ImVec2 win_size = GetContentRegionAvail();
        win_size.y -= 20;

        SetNextItemWidth(win_size.x);
        InputText("##query_spells", &query_buf);
        BeginChild("##spell_desc", ImVec2(win_size.x, -1));

        for (auto &kv : cached_results) {
            if (CollapsingHeader(kv.first.c_str())) {
                static string selected;
                Indent();
                for (auto &j : kv.second.at("results")) {
                    auto name = j.at("name").get<string>();
                    if (to_lower(name).find(to_lower(query_buf)) != string::npos ||
                        query_buf.empty()) {
                        if (Selectable(name.c_str(), name == selected)) {
                            // string path_str = "/api/spells/" + query_buf;
                            if (name == selected) {
                                selected = "";
                            } else {
                                selected = name;
                                nm.HttpGetRequest("www.dnd5eapi.co", j.at("url").get<string>());
                            }
                            http_response.clear();
                        }
                        if (name == selected) { draw_query_response(kv.first); }
                    }
                }
                Unindent();
            }
        }
        EndChild();
    }
    End();
}

void
TextWrappedF(ImFont *font, const char *fmt, ...) {
    auto    f = ImFontResource(font);
    va_list args;
    va_start(args, fmt);
    TextWrappedV(fmt, args);
    va_end(args);
}

void
UI::draw_query_response(const string &query_type) {
    // TODO support drawing other cards for api things
    string                 res;
    static NetworkManager &nm  = NetworkManager::GetInstance();
    static GUI &           gui = GUI::GetInstance();
    if (nm.HttpGetResults(res)) {
        try {
            http_response = json::parse(res);
        } catch ([[maybe_unused]] std::exception &e) {}
    }
    if (!http_response.empty()) {
        Separator();
        if (query_type == "Spells") {
            TextWrappedF(gui.MediumFont, "%s", http_response.at("name").get<string>().c_str());
            TextWrappedF(
                gui.DefaultFontIt,
                "level %d %s",
                http_response.at("level").get<int>(),
                http_response.at("school").at("name").get<string>().c_str());
            Separator();
            TextWrappedF(gui.DefaultFontBold, "%s", "Casting Time: ");
            SameLine();
            TextWrapped("%s", http_response.at("casting_time").get<string>().c_str());
            TextWrappedF(gui.DefaultFontBold, "%s", "Range: ");
            SameLine();
            TextWrapped("%s", http_response.at("range").get<string>().c_str());
            TextWrappedF(gui.DefaultFontBold, "%s", "Components: ");
            SameLine();
            string comps = "";
            for (auto &j : http_response.at("components")) { comps += j.get<string>() + " "; }
            TextWrapped(
                "%s(%s)",
                comps.c_str(),
                http_response.find("material") != http_response.end()
                    ? http_response.at("material").get<string>().c_str()
                    : "");
            TextWrappedF(gui.DefaultFontBold, "%s", "Duration: ");
            SameLine();
            TextWrapped("%s", http_response.at("duration").get<string>().c_str());
            string classes = "";
            for (auto &j : http_response.at("classes")) {
                classes += j.at("name").get<string>() + ", ";
            }
            // Get rid of the trailing comma
            classes.erase(classes.begin() + classes.length() - 2, classes.end());
            TextWrappedF(gui.DefaultFontBold, "%s", "Classes: ");
            SameLine();
            TextWrapped("%s", classes.c_str());
            string desc = "";
            for (auto &j : http_response.at("desc")) { desc += j.get<string>() + "\n"; }
            TextWrapped("%s", desc.c_str());

            if (http_response.find("higher_level") != http_response.end()) {
                TextWrappedF(gui.DefaultFontBold, "%s", "At Higher Levels");
                TextWrapped(
                    "%s",
                    http_response.at("higher_level").front().get<std::string>().c_str());
            }
        }
        Separator();
    }
}
