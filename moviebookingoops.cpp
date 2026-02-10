#include <iostream>
#include <vector>
#include <string>
#include <fstream>
#include <iomanip>
#include <algorithm>
#include <sstream>
#include <cctype>

using namespace std;

// ================= Utility =================
string trim(const string &s) {
    string str = s;
    str.erase(str.begin(), find_if(str.begin(), str.end(),
        [](unsigned char ch) { return !isspace(ch); }));
    str.erase(find_if(str.rbegin(), str.rend(),
        [](unsigned char ch) { return !isspace(ch); }).base(), str.end());
    return str;
}

// ================= MOVIE CLASS =================
class Movie {
    string title, genre;
    int duration;
    double rating;

public:
    Movie(string t = "", string g = "", int d = 0, double r = 0.0)
        : title(move(t)), genre(move(g)), duration(d), rating(r) {}

    const string& getTitle()   const { return title; }
    const string& getGenre()   const { return genre; }
    int           getDuration()const { return duration; }
    double        getRating()  const { return rating; }

    void displayInfo() const {
        cout << "Movie: " << title << " | Genre: " << genre
             << " | Duration: " << duration << " mins | Rating: " << rating << '\n';
    }

    // CSV output
    void saveCSV(ofstream &out) const {
        out << title << ',' << genre << ',' << duration << ',' << rating << '\n';
    }
};

// ================= SHOW CLASS =================
class Show {
    Movie movie;
    string showTime;
    int totalSeats;
    int bookedSeats;

public:
    Show(Movie m = Movie(), string time = "", int seats = 0)
        : movie(move(m)), showTime(move(time)), totalSeats(seats), bookedSeats(0) {}

    bool bookSeat(int count) {
        if (bookedSeats + count <= totalSeats) {
            bookedSeats += count;
            return true;
        }
        cout << "Not enough seats available!\n";
        return false;
    }

    void displayShow() const {
        cout << movie.getTitle() << " | Show Time: " << showTime
             << " | Available: " << (totalSeats - bookedSeats) << '/' << totalSeats << '\n';
    }

    const Movie& getMovie()    const { return movie; }
    const string& getShowTime()const { return showTime; }
    int getTotalSeats()        const { return totalSeats; }
    int getBookedSeats()       const { return bookedSeats; }

    // CSV output
    void saveCSV(ofstream &out) const {
        out << movie.getTitle() << ',' << showTime << ',' << totalSeats << ',' << bookedSeats << '\n';
    }
};

// ================= TICKET CLASS =================
class Ticket {
    static int counter;
    int ticketId;
    Show show;
    int seatCount;
    double price;

public:
    Ticket(Show s, int seats, double pricePerSeat)
        : show(move(s)), seatCount(seats), price(seats * pricePerSeat) {
        ticketId = counter++;
    }

    void displayTicket() const {
        cout << "\n=== Ticket #" << ticketId << " ===\n"
             << "Movie: " << show.getMovie().getTitle() << '\n'
             << "Showtime: " << show.getShowTime() << '\n'
             << "Seats: " << seatCount << " | Total Price: ₹" << price << '\n';
    }

    // CSV output
    void saveCSV(ofstream &out) const {
        out << ticketId << ',' << show.getMovie().getTitle() << ','
            << show.getShowTime() << ',' << seatCount << ',' << price << '\n';
    }
};
int Ticket::counter = 1000;

// ================= USER CLASS =================
class User {
protected:
    string name, email;
public:
    User(string n, string e) : name(move(n)), email(move(e)) {}
    virtual void showMenu() = 0;
    virtual ~User() = default;
};

// ================= ADMIN CLASS =================
class Admin : public User {
public:
    Admin(string n, string e) : User(move(n), move(e)) {}

    void saveMovies(const vector<Movie> &movies) {
        ofstream out("movies.txt");
        out << "Title,Genre,Duration,Rating\n";               // header
        for (const auto &m : movies) m.saveCSV(out);
    }

    void saveShows(const vector<Show> &shows) {
        ofstream out("shows.txt");
        out << "Movie,ShowTime,TotalSeats,Booked\n";          // header
        for (const auto &s : shows) s.saveCSV(out);
    }

    void addMovie(vector<Movie> &movies) {
        string title, genre;
        int duration;
        double rating;
        cin.ignore();
        cout << "Enter movie title: ";   getline(cin, title);
        cout << "Genre: ";               getline(cin, genre);
        cout << "Duration (mins): ";     cin >> duration;
        cout << "Rating: ";              cin >> rating;

        movies.emplace_back(move(title), move(genre), duration, rating);
        saveMovies(movies);
        cout << "Movie added successfully!\n";
    }

    void addShow(vector<Show> &shows, const vector<Movie> &movies) {
        if (movies.empty()) { cout << "No movies available. Add a movie first!\n"; return; }

        cout << "\nAvailable Movies:\n";
        for (size_t i = 0; i < movies.size(); ++i) {
            cout << i + 1 << ". "; movies[i].displayInfo();
        }

        int choice;
        cout << "Select movie number: "; cin >> choice;
        if (choice < 1 || static_cast<size_t>(choice) > movies.size()) {
            cout << "Invalid choice!\n"; return;
        }

        string time;
        int seats;
        cin.ignore();
        cout << "Enter show time (e.g. 2025-11-03 18:30): "; getline(cin, time);
        cout << "Enter total seats: ";                     cin >> seats;

        shows.emplace_back(movies[choice - 1], move(time), seats);
        saveShows(shows);
        cout << "Show added successfully!\n";
    }

    void removeMovie(vector<Movie> &movies, vector<Show> &shows) {
        string title;
        cin.ignore();
        cout << "Enter movie title to remove: "; getline(cin, title);

        movies.erase(remove_if(movies.begin(), movies.end(),
                               [&](const Movie &m) { return m.getTitle() == title; }),
                     movies.end());

        shows.erase(remove_if(shows.begin(), shows.end(),
                              [&](const Show &s) { return s.getMovie().getTitle() == title; }),
                    shows.end());

        saveMovies(movies);
        saveShows(shows);
        cout << "Movie and its shows removed successfully!\n";
    }

    void showMenu() override {
        cout << "\n=== Admin Menu ===\n"
             << "1. Add Movie\n2. Remove Movie\n3. Add Show\n4. Exit\n";
    }
};

// ================= CUSTOMER CLASS =================
class Customer : public User {
    vector<Ticket> myTickets;

public:
    Customer(string n, string e) : User(move(n), move(e)) {}

    void saveBookings() const {
        ofstream out("bookings.txt", ios::app);
        // write header only if file is empty
        out.seekp(0, ios::end);
        if (out.tellp() == 0) {
            out << "ID,Movie,ShowTime,Seats,Price\n";
        }
        for (const auto &t : myTickets) t.saveCSV(out);
    }

    void viewMovies() const {
        ifstream in("movies.txt");
        if (!in) { cout << "No movies available!\n"; return; }

        cout << "\n--- Available Movies ---\n";
        string line;
        while (getline(in, line)) cout << line << '\n';
    }

    void viewShows(const vector<Show> &shows) const {
        cout << "\n--- Available Shows ---\n";
        if (shows.empty()) { cout << "No shows available!\n"; return; }
        for (size_t i = 0; i < shows.size(); ++i) {
            cout << i + 1 << ". ";
            shows[i].displayShow();
        }
    }

    void bookTicket(vector<Show> &shows, int seatCount, double pricePerSeat) {
        if (shows.empty()) { cout << "No shows available!\n"; return; }

        int sn;
        cout << "Enter show number: "; cin >> sn;
        if (sn < 1 || static_cast<size_t>(sn) > shows.size()) {
            cout << "Invalid show number!\n"; return;
        }

        Show &chosen = shows[sn - 1];
        if (chosen.bookSeat(seatCount)) {
            Ticket t(chosen, seatCount, pricePerSeat);
            myTickets.push_back(move(t));
            cout << "Booking Successful!\n";
            myTickets.back().displayTicket();
            saveBookings();

            // rewrite shows.txt with updated booked count
            ofstream out("shows.txt");
            out << "Movie,ShowTime,TotalSeats,Booked\n";
            for (const auto &s : shows) s.saveCSV(out);
        }
    }

    void viewBookings() const {
        if (myTickets.empty()) { cout << "No bookings yet!\n"; return; }
        for (const auto &t : myTickets) t.displayTicket();
    }

    void showMenu() override {
        cout << "\n=== Customer Menu ===\n"
             << "1. View Movies\n2. View Shows\n3. Book Ticket\n"
             << "4. View My Tickets\n5. Exit\n";
    }
};

// ================= FILE LOADING (CSV) =================
vector<Movie> loadMovies() {
    vector<Movie> movies;
    ifstream in("movies.txt");
    if (!in) return movies;

    string line;
    bool header = true;
    while (getline(in, line)) {
        if (header) { header = false; continue; }           // skip header
        if (line.empty()) continue;

        stringstream ss(line);
        string title, genre, durStr, rateStr;
        getline(ss, title, ',');
        getline(ss, genre, ',');
        getline(ss, durStr, ',');
        getline(ss, rateStr, ',');

        try {
            int duration = stoi(durStr);
            double rating = stod(rateStr);
            movies.emplace_back(move(title), move(genre), duration, rating);
        } catch (...) { /* ignore malformed line */ }
    }
    return movies;
}

vector<Show> loadShows(const vector<Movie> &movies) {
    vector<Show> shows;
    ifstream in("shows.txt");
    if (!in) return shows;

    string line;
    bool header = true;
    while (getline(in, line)) {
        if (header) { header = false; continue; }
        if (line.empty()) continue;

        stringstream ss(line);
        string title, time, totalStr, bookedStr;
        getline(ss, title, ',');
        getline(ss, time, ',');
        getline(ss, totalStr, ',');
        getline(ss, bookedStr, ',');

        try {
            int totalSeats = stoi(totalStr);
            int bookedSeats = stoi(bookedStr);

            auto it = find_if(movies.begin(), movies.end(),
                              [&](const Movie &m) { return m.getTitle() == title; });
            if (it != movies.end()) {
                Show s(*it, move(time), totalSeats);
                for (int i = 0; i < bookedSeats; ++i) s.bookSeat(1);
                shows.push_back(move(s));
            }
        } catch (...) { /* ignore malformed line */ }
    }
    return shows;
}

// ================= MAIN =================
int main() {
    vector<Movie> movies = loadMovies();
    vector<Show>   shows = loadShows(movies);

    Admin   admin("Admin", "admin@movie.com");
    Customer cust("Tushar", "tushar@mail.com");

    int mode;
    cout << "Choose Mode:\n1. Admin\n2. Customer\nChoice: ";
    cin >> mode;

    if (mode == 1) {
        int ch;
        do {
            admin.showMenu();
            cout << "Enter choice: ";
            cin >> ch;
            switch (ch) {
                case 1: admin.addMovie(movies); break;
                case 2: admin.removeMovie(movies, shows); break;
                case 3: admin.addShow(shows, movies); break;
                case 4: cout << "Exiting Admin Panel...\n"; break;
                default: cout << "Invalid choice!\n";
            }
        } while (ch != 4);
    } else {
        int ch;
        do {
            cust.showMenu();
            cout << "Enter choice: ";
            cin >> ch;
            switch (ch) {
                case 1: cust.viewMovies(); break;
                case 2: cust.viewShows(shows); break;
                case 3: {
                    int seats;
                    cout << "Enter seats to book: "; cin >> seats;
                    cust.bookTicket(shows, seats, 200.0);
                    break;
                }
                case 4: cust.viewBookings(); break;
                case 5: cout << "Thank you for using Movie Booking System!\n"; break;
                default: cout << "Invalid choice!\n";
            }
        } while (ch != 5);
    }
    return 0;
}