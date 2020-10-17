// Final CMP202.cpp // James Wood /// 1902545 // Abertay University
//Comments all by me    ^^^
//With solid reference to Tutorial mandelbrot exercises.

#include <iostream>
#include <chrono>
#include <cstdint>
#include <cstdlib>
#include <complex>
#include <fstream>
#include <thread>
#include <mutex>


//Imports

using std::chrono::duration_cast;
using std::chrono::milliseconds;
using std::complex;
using std::cout;
using std::endl;
using std::ofstream;

//this mutex allows the program to access the same variables declared at the same time, this is important in our process because
//Althrough the maths will be different the program will still need data that will be the same across all the threads.
std::mutex process;


//Define Clock for time.

typedef std::chrono::steady_clock time_taken; 

//image Resolution of Mandelbrot.

const int Height = 600;
const int Width = 800;


//The number of times to iterate the Image.

const int Num_of_iterations = 500;

// Image data (0xRRGGBB).
uint32_t image[Height][Width];




//Delared output for the Mandelbrot through a .TGA file.
void write_tga(const char *outputFile)
{
	ofstream outfile(outputFile, ofstream::binary);

	uint8_t header[18] = {

		0, // no image ID.
		0, // no colour map.
		2, // uncompressed 24-bit image.
		0, 0, 0, 0, 0, // colour map specification.
		0, 0, // X Axis.
		0, 0, // Y Axis.
		Width & 0xff, (Width >> 8) & 0xFF, //Width.
		Height & 0xff, (Height >> 8) & 0xff, //Height.
		24, //bits per pixel.
		0, // Descriptor Output.

	};
	outfile.write((const char *)header, 18);

	for (int y = 0; y < Height; ++y)
	{
		for (int x = 0; x < Width; ++x)
		{
			uint8_t pixel[3] = {
				image[y][x] & 0xff,                       //Blue Colour Channel.
				(image[y][x] >> 8) & 0xff,                //Green Colour Channel.
				(image[y][x] >> 16) & 0xff,               //Red Colour Channel.  

			};
			outfile.write((const char*)pixel, 3);

		}

	}

	outfile.close();
	if (!outfile)
	{
	// Error fix for if some problem occurs.
		cout << "Error writing to" << outputFile << endl;
		exit(1);

	}
}


//Now we render the Mandelbrot into an image array:
// The parameters specify the region on the complex plane to plot.
void compute_mandelbrot(double left, double right, double top, double bottom,int startHeight,int endHeight)
{
	for (int y = startHeight; y < endHeight; ++y / 3)
	{
		for (int x = 0; x < Width; ++x / 3)
		{

			//Work out the point in the complex plane
			//corrosponds to this pixelin the output image
			complex<double> c(left + (x * (right - left) / Width),
				top + (y * (bottom - top) / Height));


			//start off z at (0, 0).
			complex<double> z(0.0, 0.0);


			// Iterate z = z^2 + c until z moves more than 2 units
			// away from (0, 0), or we've iterated too many times.
			int iterations = 0;
			while (abs(z) < 2.0 && iterations < Num_of_iterations)
			{
				z = (z * z) + c;

				++iterations;
			}
			
			if (iterations == Num_of_iterations)
			{
				process.lock();
				// z didn't escape from the circle.
				// This point is in the Mandelbrot set.
				image[y][x] = 0x800080; // Purple
				process.unlock();
			}
			else
			{
				process.lock();
				// z escaped within less than MAX_ITERATIONS
				// iterations. This point isn't in the set.
				image[y][x] = 0xFFA500; // Orange
				process.unlock();
			}
			
		}

	}

}

//Threads of work to be done.

void Thread1()
{

	compute_mandelbrot(-2.0, 1.0, 1.125, -1.125,0,Height/3);
	//Each thread does %33.33 of mandelbrot.

}

void Thread2() 
{

	compute_mandelbrot(-2.0, 1.0, 1.125, -1.125,Height / 3, (Height / 3) * 2);
	//Each thread does %33.33 of mandelbrot.

}
void Thread3()
{

	compute_mandelbrot(-2.0, 1.0, 1.125, -1.125, (Height / 3) * 2, Height);
	//Each thread does %33.33 of mandelbrot.

}










int main(int argc, char *argv[])
{
	cout << "Generating Mandelbrot...";
	// Start timing the process of making the Mandelbrot
	time_taken::time_point start = time_taken::now();
	
	
	//These are named part1, 2 and 3 because the main runs of the initial thread and does 33% of the work of the mandelbrot
	std::thread part1(Thread1);
	std::thread part2(Thread2);
	std::thread part3(Thread3);
	


	
	
	//This makes the main wait for the processes in the other threads to be done before finishing the program and outputting the result. This allows any users
	//to use the program safely.
	part1.join();
	part2.join();
	part3.join();


	// Stop timing.
	time_taken::time_point end = time_taken::now();

	// Compute the difference between the two times in milliseconds.
	auto time_taken = duration_cast<milliseconds>(end - start).count();
	cout << "Computing the Mandelbrot set took " << time_taken << " ms." << endl;

	write_tga("output.tga");

		return 0;

}

