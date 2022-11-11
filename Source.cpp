//INCLUDES
#include<cstdint>
#include<iostream>
#include<unordered_map>
#include <fstream>
#include<string>

const int32_t MaxRows = 5;
const int32_t MaxCols = 4;

struct SRect
{
	SRect(): SRect{0,0,0,0}
	{

	}
	SRect(int32_t iTop, int32_t iLeft, int32_t iWidth, int32_t iHeight)
		: Top{ iTop }
		, Left{ iLeft }
		, Width{ iWidth }
		, Height{ iHeight }
	{

	}
	int32_t Top;
	int32_t Left;
	int32_t Width;
	int32_t Height;

};

inline bool IsInvalid(const SRect& Rect)
{
	return Rect.Top < 0 || Rect.Top >= MaxRows || Rect.Height <= 0 || Rect.Height > MaxRows ||
		Rect.Left < 0 || Rect.Left >= MaxCols || Rect.Width <= 0 || Rect.Width > MaxCols;
}
//does a simple AABB check to see if rects are intersecting
inline bool Intersects(const SRect& Rect1, const SRect& Rect2)
{
	return Rect1.Top < Rect2.Top + Rect2.Height &&
		Rect1.Top + Rect1.Height > Rect2.Top &&
		Rect1.Left < Rect2.Left + Rect2.Width &&
		Rect1.Width + Rect1.Left > Rect2.Left;

}

//returns the Index of the element in the matrix, used as Key in the GridRects unordered_map
inline int32_t GetIndex(const int32_t row, const int32_t col)
{
	return (row * MaxCols) + col;
}

//returns the area of the rectangle
inline int32_t GetArea(const SRect& Rect)
{
	return Rect.Width * Rect.Height;
}

//Adds a Rect to the Grid
//input param const Rect : pass by value is intentional, to avoid Rect being deleted before it is added. 
void AddRectToGrid(const SRect Rect, std::unordered_map<int32_t, SRect>& GridRects)
{
	//check if rect is valid

	//remove and GridRects that are part of incoming rect
	const int32_t RectTop = Rect.Top;
	const int32_t RectLeft = Rect.Left;
	const int32_t RectHeight = RectTop + Rect.Height;
	const int32_t RectWidth = RectLeft + Rect.Width;
	for (int32_t i = RectTop; i < RectHeight; ++i)
	{
		for (int32_t j = RectLeft; j < RectWidth; ++j)
		{
			GridRects.erase(GetIndex(i,j));
		}
	}

	//on the remaining rects check if the rect intersects with the GridRect if no skip, 
	//else compute new largest rect
	for (auto& KVP : GridRects)
	{
		SRect& Current = KVP.second;
		if (Intersects(Current, Rect))
		{
			//RowRect
			SRect RowRect{ Current };
			RowRect.Width = Rect.Left - Current.Left;

			//ColRect
			SRect ColRect{ Current };
			ColRect.Height = Rect.Top - Current.Top;

			//pick the rect with the highest area
			Current = GetArea(RowRect) > GetArea(ColRect) ? RowRect : ColRect;
		}


	}

}

//Parses through the dictionary to return the LargestRectIndex
int32_t GetLargestRectIndexByArea(std::unordered_map<int32_t, SRect>& GridRects)
{
	int32_t LargestArea = -1;
	int32_t LargestAreaIndex = -1;

	for (auto& KVP : GridRects)
	{
		const SRect& Current = KVP.second;
		const int32_t Area = GetArea(Current);

		if (Area > LargestArea)
		{
			LargestArea = Area;
			LargestAreaIndex = KVP.first;
		}
	}

	return LargestAreaIndex;
}

//Used for Initially creating the GridRects
void InitializeGridRects(std::unordered_map<int32_t, SRect>& GridRects)
{

	for (int32_t i = 0; i < MaxRows; ++i)
	{
		for (int32_t j = 0; j < MaxCols; ++j)
		{
			const int32_t Height = MaxRows - i;
			const int32_t Width = MaxCols - j;
			GridRects[GetIndex(i, j)] = SRect{ i, j, Width, Height };
		}
	}
}

//Converts the Input File lines into Rectangles
bool ProcessInputFile(std::vector<std::string>& StrRects, std::vector<SRect>& InputRects)
{
	bool bSuccess = false;

	for (auto& Line : StrRects)
	{
		//remove whitespace
		Line.erase(std::remove_if(Line.begin(), Line.end(), isspace), Line.end());
	
		int32_t Top, Left, Width, Height;
		size_t Start = 0, Pos = 0;

		Pos = Line.find(',', Start);
		Top = std::stoi(Line.substr(Start,Pos - Start));

		Start	= Pos + 1;
		Pos		= Line.find(',', Start);
		Left	= std::stoi(Line.substr(Start, Pos - Start));

		Start	= Pos + 1;
		Pos		= Line.find(',', Start);
		Width	= std::stoi(Line.substr(Start, Pos - Start));

		Start	= Pos + 1;
		Pos		= Line.find(',', Start);
		Height	= std::stoi(Line.substr(Start, Pos - Start));
		SRect InputRect{ Top,Left,Width,Height };
		if (IsInvalid(InputRect))
		{
			return bSuccess;
		}
		InputRects.push_back(SRect{ Top,Left,Width,Height });
	}
	bSuccess = InputRects.size() == StrRects.size();
	return bSuccess;
}

//Loads the file from the hard drive
bool LoadFile(std::string& InputFilePath, std::vector<std::string>& Lines)
{
	bool bSuccess = false;
	std::ifstream InputFile{ InputFilePath };
	if (!InputFile.is_open())
	{
		return bSuccess;
	}

	std::string Line;
	while (std::getline(InputFile, Line))
	{
		Lines.push_back(Line);
	}
	InputFile.close();

	bSuccess = !Lines.empty();
	return bSuccess;
}


int32_t main(int32_t argc, char** argv)
{

	//Process command args
	if (argc < 2)
	{
		std::cout << "Please provide input file as command line argument" << std::endl;
		return -1;
	}
	if (argc > 2)
	{
		std::cout << "Please provide only 1 input file at a time" << std::endl;
		return -1;
	}

	std::vector<SRect> InputRects;
	std::string InputFilePath{};
	if (argc >= 2)
	{
		//Load File into Memory
		InputFilePath = argv[argc - 1];
		std::vector<std::string> Lines;
		if (!LoadFile(InputFilePath, Lines))
		{
			std::cout << "Invalid InputFile" << std::endl;
			return -1;
		}
		//Process File
		if (!ProcessInputFile(Lines, InputRects))
		{
			std::cout << "Invalid data in InputFile" << std::endl;
			return -1;
		}

	}

	std::unordered_map<int32_t, SRect> GridRects;
	InitializeGridRects(GridRects);

	//add the file rects into the Grid
	for (auto& rect : InputRects)
	{
		AddRectToGrid(rect, GridRects);
	}

	//now we start adding our biggest rects until there is no rects remaining in the Grid
	int32_t TotalMinimumRects = 0;
	while (!GridRects.empty())
	{
		const int32_t LargestRectIndex = GetLargestRectIndexByArea(GridRects);

		AddRectToGrid(GridRects[LargestRectIndex], GridRects);
		++TotalMinimumRects;
	}

	std::cout << "InputFile: " << InputFilePath << std::endl;
	std::cout << "Minimum Rects: " << TotalMinimumRects << std::endl;

	return 0;
}