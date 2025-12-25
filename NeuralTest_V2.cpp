#include<iostream>
#include<thread>
#include<atomic>
#include<chrono>
#include<cmath>
#define rad 57.2958

using Clock = std::chrono::high_resolution_clock;
using us = std::chrono::microseconds;

using namespace std;

//set time delay function - definition
void delay(int t)             
{
    auto start = Clock::now();
    auto end = start + t * 1us;
    while (chrono::high_resolution_clock::now() < end)
    {
        //Eat 5 - Star, do nothing
        this_thread::yield();//allows permission for other tasks to run while waiting
    } 
}

atomic<int> NEURON_L(180);//Initialisation of arbitrary choice
atomic<int> NEURON_R(1);
atomic<int> NEURON_FIRED(1);//CENTRAL thread
atomic<int> QUADRANT(0); //For side
atomic<int> ITD(0); //Inter-Aural Time Difference, not needed to be atomic though...

//function modules definitions
inline double dist(double xi, double xf, double yi, double yf)
{
    return sqrt(pow(xf - xi, 2) + pow(yf - yi, 2));
}
inline double len(double x, double y)
{
    return sqrt(x * x + y * y);
}

void R()
{
    delay(100);//TO SYNCHRONISE
    if (QUADRANT == 2)
    {
        //Wait(ITD)
        delay(ITD);
    }
    //Proceed
    while (NEURON_R.load() < 180)
    {
        NEURON_R++;
        delay(1000 * 3.24);//scaled
    }
}

void L()
{
    delay(100);//TO SYNCHRONISE
    if (QUADRANT == 1)
    {
        //Wait(ITD);
        delay(ITD);
    }
    while (NEURON_L.load() > 1)
    {
        NEURON_L--;
        delay(1000 * 3.24);//scaled
    }
}

void CentralNeurons()
{
    delay(100);//TO SYNCHRONISE
    while (true)
    {
        delay(100);//Prevent jitter
        if (abs(NEURON_L.load() - NEURON_R.load()) <= 1)
        {
            NEURON_FIRED = NEURON_R.load();
            break;//Coincidence detected
        }
    }
    cout << "Central Neuron Fired at Neuron Line: " << NEURON_FIRED << endl;
}
int main()
{
    while (true) 
    {
        double deg = 0;
        double r = 100; //radius in cm, source to origin
        double c = 0.0343; //Set speed of sound as c, in cm per microsecond

        cout << "\nEnter Azimuth (0 < deg < 180) : ";
        cin >> deg;
        if (deg <= 90 && deg >= 0)
        {
            QUADRANT = 1; //RIGHT, 1st Quadrant flag
        }
        else if (deg > 90 && deg <= 180)
        {
            QUADRANT = 2; //2nd Quadrant flag
        }
        else
        {
            cout << "Input out of bound" << endl;
            continue;
        }

        //Pre-Calculating distances -> ITD :
        double x = r * cos(deg / rad);  double y = r * sin(deg / rad); //Position of Source(x,y)
        double _Source_to_LeftMic_l = len(x - (-10), y - 0); //since left Microphone position(-10,0)
        double _Source_to_RightMic_l = len(x - 10, y - 0);  //since right Microphone position(10,0)
        double _time_to_LeftMic = _Source_to_LeftMic_l / c;
        double _time_to_RightMic = _Source_to_RightMic_l / c;
        ITD = 1000 * (int)abs(_time_to_LeftMic - _time_to_RightMic); //Absolute difference, scaled to 100
        cout << "\nITD : " << ITD << " us" << endl;

        //SIMULATION starts here
        thread Left(L);
        thread Neurons(CentralNeurons);
        thread Right(R);
        Left.join();
        Neurons.join();
        Right.join();

        // Reset shared state
        NEURON_L = 180;
        NEURON_R = 1;
        NEURON_FIRED = 1;
    }
    return 0;
}