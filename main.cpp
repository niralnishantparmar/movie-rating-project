// ============================================================
// Movie Rating System
// Grade 11 C++ Project
// Team: ____________________ & ____________________
//
// This program manages a list of movies and their ratings
// (0-10). Each movie is stored as an object of the Movie class.
// A menu lets the user view, add, search and sort movies, and
// see a summary with the average, highest and lowest ratings.
// Each movie is labeled RECOMMENDED, MIXED REVIEWS or
// NOT RECOMMENDED, shown with a color and a star bar.
// ============================================================

#include <iostream>
#include <string>
#include <vector>
#include <iomanip>   // setprecision for clean decimals
#include <sstream>   // ostringstream to format numbers as text
#include <limits>    // numeric_limits for clearing bad input
#include <cctype>    // tolower for case-insensitive search
#include <cmath>     // floor
#include <algorithm> // stable_sort

// ---- GUI includes (ImGui + browser) ----
#include <emscripten.h>
#include <GLFW/glfw3.h>
#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"

using namespace std;

// ------------------------------------------------------------
// DISPLAY SETTINGS
// Set USE_COLOR to false if your console shows strange
// symbols like [32m instead of colors.
// ------------------------------------------------------------
const bool USE_COLOR = true;

// ANSI color codes (special text the console turns into colors)
const string RESET  = "\033[0m";
const string BOLD   = "\033[1m";
const string RED    = "\033[31m";
const string GREEN  = "\033[32m";
const string YELLOW = "\033[33m";
const string CYAN   = "\033[36m";

const int BOX_WIDTH = 66;   // inner width of the boxes

// Returns text wrapped in a color (or plain text if colors are off)
string paint(string text, string colorCode) {
    if (!USE_COLOR) {
        return text;
    }
    return colorCode + text + RESET;
}

// Adds spaces to the end of text so columns line up.
// Long text is shortened with "..." so it does not break the table.
string padRight(string text, int width) {
    if ((int)text.length() > width - 1) {
        text = text.substr(0, width - 4) + "...";
    }
    while ((int)text.length() < width) {
        text += " ";
    }
    return text;
}

// Turns a number like 9.2 into the text "9.2"
string formatRating(double value) {
    ostringstream out;
    out << fixed << setprecision(1) << value;
    return out.str();
}

// Returns a lowercase copy of text (used for searching)
string toLowerCase(string text) {
    for (int i = 0; i < (int)text.length(); i++) {
        text[i] = tolower(text[i]);
    }
    return text;
}

// ---------- Box drawing helpers (the "frontend") ----------

// Prints a border line like +-----------+
void printBorder() {
    cout << paint("+" + string(BOX_WIDTH, '-') + "+", CYAN) << endl;
}

// Prints one row inside a box: | text        |
void printBoxRow(string text) {
    cout << paint("|", CYAN) << " " << padRight(text, BOX_WIDTH - 1)
         << paint("|", CYAN) << endl;
}

// Prints a centered, bold title row inside a box
void printBoxTitle(string text) {
    int leftSpace = (BOX_WIDTH - (int)text.length()) / 2;
    int rightSpace = BOX_WIDTH - (int)text.length() - leftSpace;
    cout << paint("|", CYAN) << string(leftSpace, ' ')
         << paint(text, BOLD) << string(rightSpace, ' ')
         << paint("|", CYAN) << endl;
}

// ------------------------------------------------------------
// Movie class: stores one movie's title and rating
// ------------------------------------------------------------
class Movie {
private:
    // Private data: can only be reached through the methods below
    string title;
    double rating;

public:
    // Parameterized constructor: assigns the title and rating
    // when each Movie object is created
    Movie(string movieTitle, double movieRating) {
        title = movieTitle;
        rating = movieRating;
    }

    // Getter methods
    string getTitle() {
        return title;
    }

    double getRating() {
        return rating;
    }

    // Decides the recommendation status using if / else
    string getStatus() {
        if (rating >= 7.0) {
            return "RECOMMENDED";
        } else if (rating >= 5.0) {
            return "MIXED REVIEWS";
        } else {
            return "NOT RECOMMENDED";
        }
    }

    // Picks a color that matches the status
    string getStatusColor() {
        if (rating >= 7.0) {
            return GREEN;
        } else if (rating >= 5.0) {
            return YELLOW;
        } else {
            return RED;
        }
    }

    // Builds a star bar out of 10, e.g. 9.2 -> [*********.]
    string getStars() {
        int filled = (int)(rating + 0.5);   // round to nearest whole number
        string stars = "";
        for (int i = 0; i < 10; i++) {
            if (i < filled) {
                stars += "*";
            } else {
                stars += ".";
            }
        }
        return "[" + stars + "]";
    }

    // Displays this movie as one colored row of the table
    void display(int number) {
        cout << paint("|", CYAN) << " "
             << padRight(to_string(number), 4)
             << padRight(title, 24)
             << padRight(formatRating(rating), 7)
             << paint(getStars(), YELLOW) << "  "
             << paint(padRight(getStatus(), 16), getStatusColor())
             << paint("|", CYAN) << endl;
    }
};

// ------------------------------------------------------------
// Input helpers: keep asking until the input is valid
// ------------------------------------------------------------

// Reads a whole number between minValue and maxValue
int getValidInt(string prompt, int minValue, int maxValue) {
    int value;
    while (true) {
        cout << prompt;
        cin >> value;

        if (cin.fail()) {
            // User typed letters instead of a number
            cin.clear();
            cin.ignore(numeric_limits<streamsize>::max(), '\n');
            cout << paint("  Invalid input. Please enter a number.", RED) << endl;
        } else if (value < minValue || value > maxValue) {
            cin.ignore(numeric_limits<streamsize>::max(), '\n');
            cout << paint("  Please enter a number from " + to_string(minValue)
                          + " to " + to_string(maxValue) + ".", RED) << endl;
        } else {
            // Remove the leftover newline so getline works next time
            cin.ignore(numeric_limits<streamsize>::max(), '\n');
            return value;
        }
    }
}

// Reads a rating between 0 and 10 (decimals allowed)
double getValidRating() {
    double value;
    while (true) {
        cout << "  Enter rating (0 - 10): ";
        cin >> value;

        if (cin.fail()) {
            cin.clear();
            cin.ignore(numeric_limits<streamsize>::max(), '\n');
            cout << paint("  Invalid input. Please enter a number.", RED) << endl;
        } else if (value < 0 || value > 10) {
            cin.ignore(numeric_limits<streamsize>::max(), '\n');
            cout << paint("  Rating must be between 0 and 10.", RED) << endl;
        } else {
            cin.ignore(numeric_limits<streamsize>::max(), '\n');
            return value;
        }
    }
}

// ------------------------------------------------------------
// Menu actions
// ------------------------------------------------------------

// Prints the column headings for the movie table
void printTableHeader() {
    printBorder();
    cout << paint("|", CYAN) << " "
         << paint(padRight("#", 4) + padRight("Title", 24) + padRight("Rating", 7)
                  + padRight("Stars", 14) + padRight("Status", 16), BOLD)
         << paint("|", CYAN) << endl;
    printBorder();
}

// Option 1: show every movie in a table
void viewAll(vector<Movie>& movies) {
    cout << endl;
    printTableHeader();
    for (int i = 0; i < (int)movies.size(); i++) {
        movies[i].display(i + 1);
    }
    printBorder();
}

// Option 2: create a new Movie object from user input
void addMovie(vector<Movie>& movies) {
    string title = "";
    cout << endl;
    while (title == "") {
        cout << "  Enter title: ";
        getline(cin, title);   // getline allows titles with spaces
    }
    double rating = getValidRating();

    // Create the object with the parameterized constructor
    Movie newMovie(title, rating);
    movies.push_back(newMovie);

    cout << paint("  Added \"" + title + "\" -> " + newMovie.getStatus(),
                  newMovie.getStatusColor()) << endl;
}

// Option 3: find movies whose title contains the search words
void searchMovie(vector<Movie>& movies) {
    string search;
    cout << endl << "  Enter part of a title to search: ";
    getline(cin, search);

    int found = 0;
    printTableHeader();
    for (int i = 0; i < (int)movies.size(); i++) {
        // Compare in lowercase so "lion" matches "The Lion King"
        if (toLowerCase(movies[i].getTitle()).find(toLowerCase(search)) != string::npos) {
            movies[i].display(i + 1);
            found++;
        }
    }
    if (found == 0) {
        printBoxRow("No movie found matching \"" + search + "\".");
    }
    printBorder();
}

// Option 4: sort movies from highest to lowest rating (bubble sort)
void sortByRating(vector<Movie>& movies) {
    int n = movies.size();
    for (int i = 0; i < n - 1; i++) {
        for (int j = 0; j < n - 1 - i; j++) {
            // Swap if the left movie has a lower rating than the right one
            if (movies[j].getRating() < movies[j + 1].getRating()) {
                Movie temp = movies[j];
                movies[j] = movies[j + 1];
                movies[j + 1] = temp;
            }
        }
    }
    cout << endl << paint("  Movies sorted from highest to lowest rating.", GREEN) << endl;
    viewAll(movies);
}

// Option 5: calculate and show the summary statistics
void showSummary(vector<Movie>& movies) {
    double total = 0.0;
    int bestIndex = 0;
    int worstIndex = 0;
    int recommended = 0;
    int mixed = 0;
    int notRecommended = 0;

    // Loop through every movie to total, count and compare
    for (int i = 0; i < (int)movies.size(); i++) {
        double r = movies[i].getRating();
        total += r;

        if (r > movies[bestIndex].getRating()) {
            bestIndex = i;
        }
        if (r < movies[worstIndex].getRating()) {
            worstIndex = i;
        }

        if (movies[i].getStatus() == "RECOMMENDED") {
            recommended++;
        } else if (movies[i].getStatus() == "MIXED REVIEWS") {
            mixed++;
        } else {
            notRecommended++;
        }
    }

    // Basic calculation: average rating
    double average = total / movies.size();
    ostringstream avgText;
    avgText << fixed << setprecision(2) << average;

    cout << endl;
    printBorder();
    printBoxTitle("SUMMARY");
    printBorder();
    printBoxRow("Total movies:      " + to_string(movies.size()));
    printBoxRow("Average rating:    " + avgText.str());
    printBoxRow("Highest rated:     " + movies[bestIndex].getTitle()
                + " (" + formatRating(movies[bestIndex].getRating()) + ")");
    printBoxRow("Lowest rated:      " + movies[worstIndex].getTitle()
                + " (" + formatRating(movies[worstIndex].getRating()) + ")");
    printBoxRow("Recommended:       " + to_string(recommended));
    printBoxRow("Mixed reviews:     " + to_string(mixed));
    printBoxRow("Not recommended:   " + to_string(notRecommended));
    printBorder();

    // Overall verdict based on the average
    if (average >= 7.0) {
        cout << paint("  Overall: Great movie list!", GREEN) << endl;
    } else if (average >= 5.0) {
        cout << paint("  Overall: Decent list with some mixed picks.", YELLOW) << endl;
    } else {
        cout << paint("  Overall: This list needs better movies.", RED) << endl;
    }
}

// Prints the main menu inside a box
void printMenu() {
    cout << endl;
    printBorder();
    printBoxTitle("MAIN MENU");
    printBorder();
    printBoxRow("1. View all movies");
    printBoxRow("2. Add a movie");
    printBoxRow("3. Search for a movie");
    printBoxRow("4. Sort by rating (highest first)");
    printBoxRow("5. Show summary");
    printBoxRow("6. Exit");
    printBorder();
}

// ------------------------------------------------------------
// consoleMain: the original console version (kept for the repo,
// not used by the web build). Rename it back to main() and remove
// the GUI section below to run the console version again.
// ------------------------------------------------------------
int consoleMain() {
    vector<Movie> movies;   // stores all Movie objects

    // Create 3 starting objects with the parameterized constructor
    movies.push_back(Movie("The Lion King", 9.2));
    movies.push_back(Movie("Cats", 2.8));
    movies.push_back(Movie("The Martian", 6.5));

    // Title banner
    printBorder();
    printBoxTitle("MOVIE RATING SYSTEM");
    printBoxTitle("Rate it. Rank it. Pick your next movie.");
    printBorder();

    int choice;

    // Main loop: keeps showing the menu until the user picks 6
    do {
        printMenu();
        choice = getValidInt("Enter your choice (1-6): ", 1, 6);

        switch (choice) {
            case 1:
                viewAll(movies);
                break;
            case 2:
                addMovie(movies);
                break;
            case 3:
                searchMovie(movies);
                break;
            case 4:
                sortByRating(movies);
                break;
            case 5:
                showSummary(movies);
                break;
            case 6:
                cout << endl << paint("  Thank you for using the Movie Rating System!", GREEN) << endl;
                break;
        }
    } while (choice != 6);

    return 0;
}

// ============================================================
// ImGui frontend (runs in the browser via WebAssembly)
// ============================================================

// Same green / yellow / red logic as Movie::getStatusColor, as ImGui colors
static ImVec4 statusColorUI(double r) {
    if (r >= 7.0) return ImVec4(0.30f, 0.85f, 0.45f, 1);
    if (r >= 5.0) return ImVec4(0.95f, 0.80f, 0.25f, 1);
    return ImVec4(0.95f, 0.35f, 0.35f, 1);
}

static void drawUI(vector<Movie>& movies) {
    static char titleBuf[128] = "";
    static char searchBuf[128] = "";
    static float ratingInput = 7.0f;
    static string message = "";
    static ImVec4 messageColor = ImVec4(1, 1, 1, 1);

    ImGuiIO& io = ImGui::GetIO();
    ImGui::SetNextWindowPos(ImVec2(0, 0));
    ImGui::SetNextWindowSize(io.DisplaySize);
    ImGui::Begin("Movie Rating System", nullptr,
                 ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize |
                 ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoCollapse);
    ImGui::SetWindowFontScale(1.4f);

    ImGui::TextColored(ImVec4(0.4f, 0.8f, 1.0f, 1), "MOVIE RATING SYSTEM");
    ImGui::TextDisabled("Rate it. Rank it. Pick your next movie.");
    ImGui::Separator();

    if (ImGui::BeginTabBar("tabs")) {

        // ---------- Movies tab (view + search + sort) ----------
        if (ImGui::BeginTabItem("Movies")) {
            ImGui::SetNextItemWidth(300);
            ImGui::InputTextWithHint("##search", "Search title...", searchBuf, 128);
            ImGui::SameLine();
            if (ImGui::Button("Sort by rating")) {
                // Movie passed by value (copies), so the non-const getters work
                stable_sort(movies.begin(), movies.end(),
                    [](Movie a, Movie b) { return a.getRating() > b.getRating(); });
            }
            ImGui::Spacing();

            if (ImGui::BeginTable("movies", 5, ImGuiTableFlags_RowBg | ImGuiTableFlags_Borders |
                                  ImGuiTableFlags_ScrollY)) {
                ImGui::TableSetupColumn("#", ImGuiTableColumnFlags_WidthFixed, 40);
                ImGui::TableSetupColumn("Title", ImGuiTableColumnFlags_WidthStretch);
                ImGui::TableSetupColumn("Rating", ImGuiTableColumnFlags_WidthFixed, 80);
                ImGui::TableSetupColumn("Stars", ImGuiTableColumnFlags_WidthFixed, 220);
                ImGui::TableSetupColumn("Status", ImGuiTableColumnFlags_WidthFixed, 200);
                ImGui::TableSetupScrollFreeze(0, 1);
                ImGui::TableHeadersRow();

                string needle = toLowerCase(searchBuf);
                for (int i = 0; i < (int)movies.size(); i++) {
                    if (toLowerCase(movies[i].getTitle()).find(needle) == string::npos) continue;
                    double r = movies[i].getRating();
                    ImGui::TableNextRow();
                    ImGui::TableNextColumn(); ImGui::Text("%d", i + 1);
                    ImGui::TableNextColumn(); ImGui::TextUnformatted(movies[i].getTitle().c_str());
                    ImGui::TableNextColumn(); ImGui::Text("%s", formatRating(r).c_str());
                    ImGui::TableNextColumn();
                    ImGui::PushStyleColor(ImGuiCol_PlotHistogram, statusColorUI(r));
                    ImGui::ProgressBar((float)(r / 10.0), ImVec2(-1, 0), "");
                    ImGui::PopStyleColor();
                    ImGui::TableNextColumn();
                    ImGui::TextColored(statusColorUI(r), "%s", movies[i].getStatus().c_str());
                }
                ImGui::EndTable();
            }
            ImGui::EndTabItem();
        }

        // ---------- Add tab ----------
        if (ImGui::BeginTabItem("Add Movie")) {
            ImGui::Spacing();
            ImGui::SetNextItemWidth(400);
            ImGui::InputText("Title", titleBuf, 128);
            ImGui::SetNextItemWidth(400);
            ImGui::SliderFloat("Rating", &ratingInput, 0.0f, 10.0f, "%.1f");

            double preview = floor(ratingInput * 10 + 0.5) / 10;
            Movie previewMovie("", preview);
            ImGui::TextColored(statusColorUI(preview), "Status: %s", previewMovie.getStatus().c_str());
            ImGui::Spacing();

            if (ImGui::Button("Add movie")) {
                string t = titleBuf;
                size_t start = t.find_first_not_of(' ');
                if (start == string::npos) {
                    message = "Please enter a title.";
                    messageColor = ImVec4(0.95f, 0.35f, 0.35f, 1);
                } else {
                    t = t.substr(start);
                    Movie newMovie(t, preview);   // parameterized constructor
                    movies.push_back(newMovie);
                    message = "Added \"" + t + "\" -> " + newMovie.getStatus();
                    messageColor = statusColorUI(preview);
                    titleBuf[0] = '\0';
                }
            }
            if (!message.empty()) ImGui::TextColored(messageColor, "%s", message.c_str());
            ImGui::EndTabItem();
        }

        // ---------- Summary tab ----------
        if (ImGui::BeginTabItem("Summary")) {
            ImGui::Spacing();
            if (movies.empty()) {
                ImGui::Text("No movies yet.");
            } else {
                double total = 0;
                int best = 0, worst = 0, rec = 0, mixed = 0, notRec = 0;
                for (int i = 0; i < (int)movies.size(); i++) {
                    double r = movies[i].getRating();
                    total += r;
                    if (r > movies[best].getRating()) best = i;
                    if (r < movies[worst].getRating()) worst = i;
                    string s = movies[i].getStatus();
                    if (s == "RECOMMENDED") rec++;
                    else if (s == "MIXED REVIEWS") mixed++;
                    else notRec++;
                }
                double avg = total / movies.size();
                ImGui::Text("Total movies:    %d", (int)movies.size());
                ImGui::Text("Average rating:  %.2f", avg);
                ImGui::Text("Highest rated:   %s (%s)", movies[best].getTitle().c_str(),
                            formatRating(movies[best].getRating()).c_str());
                ImGui::Text("Lowest rated:    %s (%s)", movies[worst].getTitle().c_str(),
                            formatRating(movies[worst].getRating()).c_str());
                ImGui::TextColored(statusColorUI(8), "Recommended:     %d", rec);
                ImGui::TextColored(statusColorUI(6), "Mixed reviews:   %d", mixed);
                ImGui::TextColored(statusColorUI(2), "Not recommended: %d", notRec);
                ImGui::Spacing();
                const char* verdict = avg >= 7.0 ? "Overall: Great movie list!"
                                    : avg >= 5.0 ? "Overall: Decent list with some mixed picks."
                                                 : "Overall: This list needs better movies.";
                ImGui::TextColored(statusColorUI(avg), "%s", verdict);
            }
            ImGui::EndTabItem();
        }
        ImGui::EndTabBar();
    }
    ImGui::End();
}

static GLFWwindow* gWindow;
static vector<Movie> gMovies;

// Runs once per browser frame
static void frame() {
    // Keep the canvas the same size as the browser window
    int bw = EM_ASM_INT({ return window.innerWidth; });
    int bh = EM_ASM_INT({ return window.innerHeight; });
    int cw, ch;
    glfwGetWindowSize(gWindow, &cw, &ch);
    if (cw != bw || ch != bh) {
        glfwSetWindowSize(gWindow, bw, bh);
    }

    glfwPollEvents();
    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplGlfw_NewFrame();
    ImGui::NewFrame();
    drawUI(gMovies);
    ImGui::Render();

    int w, h;
    glfwGetFramebufferSize(gWindow, &w, &h);
    glViewport(0, 0, w, h);
    glClearColor(0.08f, 0.08f, 0.10f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
    glfwSwapBuffers(gWindow);
}

int main() {
    // Starting movies (same as the console version)
    gMovies.push_back(Movie("The Lion King", 9.2));
    gMovies.push_back(Movie("Cats", 2.8));
    gMovies.push_back(Movie("The Martian", 6.5));

    // Make the auto-generated page show ONLY the app:
    // dark background, no margins, canvas covers the whole window
    EM_ASM({
        document.title = "Movie Rating System";
        document.body.style.cssText = "margin:0;overflow:hidden;background:#141419;";
        var c = document.getElementById('canvas');
        c.style.cssText = "position:fixed;top:0;left:0;width:100vw;height:100vh;z-index:9999;display:block;border:0;";
    });

    glfwInit();
    gWindow = glfwCreateWindow(1000, 700, "Movie Rating System", nullptr, nullptr);
    glfwMakeContextCurrent(gWindow);

    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGui::GetIO().IniFilename = nullptr;
    ImGui::StyleColorsDark();
    ImGui_ImplGlfw_InitForOpenGL(gWindow, true);
    ImGui_ImplGlfw_InstallEmscriptenCallbacks(gWindow, "#canvas");
    ImGui_ImplOpenGL3_Init("#version 300 es");

    emscripten_set_main_loop(frame, 0, true);
    return 0;
}