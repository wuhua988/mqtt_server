#include <iostream>
#include <vector>
#include <numeric>
#include <chrono>

#include <sstream>
#include <iomanip>
#include <ctime>

volatile int sink;

uint32_t gettimeofday()
{
    return (uint32_t) std::time(nullptr);
    //auto now = std::chrono::system_clock::now();
    //return std::chrono::system_clock::to_time_t(now);
}

// IS0 8601 time format
std::string format_time(uint32_t time)
{
    std::time_t tm_value = (std::time_t)time;
    std::stringstream ss;

    ss << std::put_time(std::localtime(&tm_value), "%F %T");

    std::string str = ss.str();

    return str;
}

int main()
{
    uint32_t cur_tm = gettimeofday();

    std::cout << "Cur time: " << cur_tm << std::endl;
    std::cout << "Cur time: " << format_time(cur_tm) << std::endl;

    for (auto size = 1ull; size < 1000000000ull; size *= 100)
    {
                // record start time
                auto start = std::chrono::system_clock::now();
                //
                // do some work
                std::vector<int> v(size, 42);
                sink = std::accumulate(v.begin(), v.end(), 0u); // make sure it's a side effect

                // record end time
                auto end = std::chrono::system_clock::now();
                std::chrono::duration<double> diff = end-start;
                std::cout << "Time to fill and iterate a vector of " << size << " ints : " << diff.count() << " s\n";
    }

}
