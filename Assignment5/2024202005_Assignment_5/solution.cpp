#include <iostream>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <ctime>  // For time functions

using namespace std;

struct Student {
    int arrival_time;
    int wash_time;
    int patience;
    int id;
};

mutex mtx;
condition_variable cv;
int available_machines;
int left_unwashed_count = 0;
mutex left_unwashed_mutex;

void rearrange(Student* students, int num_students) {
    for (int i = 0; i < num_students - 1; ++i) {
        int min_index = i;
        for (int j = i + 1; j < num_students; ++j) {
            if ((students[j].arrival_time < students[min_index].arrival_time) ||
                (students[j].arrival_time == students[min_index].arrival_time && students[j].id < students[min_index].id)) {
                min_index = j;
            }
        }
        if (min_index != i) {
            Student temp = students[i];
            students[i] = students[min_index];
            students[min_index] = temp;
        }
    }
}

void simulate_student(Student student) {
    // Use time_t to sleep instead of chrono
    time_t arrival_time = time(0);
    time_t end_time = arrival_time + student.arrival_time;

    // Busy-wait loop to simulate arrival time using time functions (since we can't use sleep_for)
    while (time(0) < end_time) {
        // Just waiting, no-op.
    }

    unique_lock<mutex> lock(mtx);
    cout << "Student " << to_string(student.id) << " arrives\n" << flush;

    time_t patience_end_time = time(0) + student.patience;
    if (cv.wait_for(lock, chrono::seconds(student.patience), []() {
        return available_machines > 0;
    })) {
        --available_machines;
        cout << "\033[1;32m" << "Student " << to_string(student.id) << " starts washing\n" << "\033[0m" << flush;
        lock.unlock();
        
        time_t wash_end_time = time(0) + student.wash_time;
        while (time(0) < wash_end_time) {
            // Simulate washing time
        }

        lock.lock();
        cout << "\033[1;33m" << "Student " << to_string(student.id) << " leaves after washing\n" << "\033[0m" << flush;
        ++available_machines;
        cv.notify_all();
    } else {
        unique_lock<mutex> left_lock(left_unwashed_mutex);
        cout << "\033[1;31m" << "Student " << to_string(student.id) << " leaves without washing\n" << "\033[0m" << flush;

        ++left_unwashed_count;
    }
}

void start_simulation(int num_students, int num_machines, Student* students) {
    available_machines = num_machines;

    rearrange(students, num_students);

    thread* threads = new thread[num_students];
    for (int i = 0; i < num_students; ++i) {
        threads[i] = thread(simulate_student, students[i]);
    }
    for (int i = 0; i < num_students; ++i) {
        threads[i].join();
    }
    delete[] threads;
}

int main() {
    int n, m;
    cin >> n >> m;

    Student* students = new Student[n];
    for (int i = 0; i < n; ++i) {
        int T_i, W_i, P_i;
        cin >> T_i >> W_i >> P_i;
        students[i] = { T_i, W_i, P_i, i + 1 };
    }
    rearrange(students,n);
    start_simulation(n, m, students);
    cout << left_unwashed_count << "\n";

    if ((left_unwashed_count) * 100 / n >= 25) {
        cout << "Yes\n";
    } else {
        cout << "No\n";
    }

    delete[] students;
    return 0;
}
