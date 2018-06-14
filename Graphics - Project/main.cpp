#if defined(UNICODE) && !defined(_UNICODE)
    #define _UNICODE
#elif defined(_UNICODE) && !defined(UNICODE)
    #define UNICODE
#endif

#include <tchar.h>
#include <windows.h>
#include "menu_header.h"
#include <math.h>
#include <stack>
#include <vector>
#include <iostream>
#include <fstream>
#include <stdlib.h>



using namespace std;

/*  Declare Windows procedure  */
LRESULT CALLBACK WindowProcedure (HWND, UINT, WPARAM, LPARAM);

bool firstpoint = false, secondpoint = false, thirdpoint = false, fourthpoint = false, ClipPoint1 = false, ClipPoint2 = false;
int menuItemID, Xleft, Xright, Ytop, Ybottom;

bool StraightLine = false;
bool DDALine = false;
bool MidpointLine = false;
bool MidpointCircle = false;
bool DirectCircle =false;
bool PolarCircle = false;
bool SetFill = false;
bool ClipLine = false;
bool ClipPolygon = false;
bool Bezier = false;
bool Hermit = false;
bool load = false;
bool polygon = false;

struct Point
{
    int x, y;
};

Point First, Second, Third, Fourth;

//save vectors
vector<Point> Lines;
vector<Point> ClippedLines;
vector<Point> Circle;
vector<Point> Fill;
vector<Point> Curves;


//load vectors
vector<Point> LoadLines;
vector<Point> LoadCircles;
vector<Point> LoadFill;
vector<Point> LoadCurve;




/*  Make the class name into a global variable  */
TCHAR szClassName[ ] = _T("CodeBlocksWindowsApp");


int round(int a)
{
    return (int)(a + 0.5);
}

void Swap(int &a, int &b)
{

    int temp;
    temp = a;
    a = b;
    b = temp;
}

void DrawSimpleLine(HDC hdc, Point Begin, Point Finish, COLORREF colorref)
{
    int Xs, Ys, Xe, Ye;
    Xs = Begin.x;
    Ys = Begin.y;
    Xe = Finish.x;
    Ye = Finish.y;
    int dx = Xe - Xs;
    int dy = Ye - Ys;
    if (abs(dy) <= abs(dx))
    {
        double m = (double)dy/dx;
        if ( Xs > Xe)
        {
            Swap(Xs, Xe);
            Swap(Ys, Ye);
        }
        for (int i = Xs; i <= Xe; i++)
        {
            int y = round(Ys + (i - Xs)*m);
            SetPixel(hdc, i, y, colorref);
        }
    }
    else
    {
        double m = (double)dx/dy;
        if ( Ys > Ye )
        {
            Swap(Xs, Xe);
            Swap(Ys, Ye);
        }
        for(int i = Ys; i <= Ye; i++)
        {
            int x = round(Xs + (i - Ys)*m);
            SetPixel(hdc, x, i, colorref);
        }
    }
}

void DrawDDALine(HDC hdc, Point Begin, Point Finish, COLORREF colorref)
{
    int Xs, Ys, Xe, Ye;
    Xs = Begin.x;
    Ys = Begin.y;
    Xe = Finish.x;
    Ye = Finish.y;
    int dy = Ye - Ys;
    int dx = Xe - Xs;
    SetPixel(hdc, Xs, Ys, colorref);
    if ( abs(dx) >= abs(dy))
    {
        int X = Xs, Xinc;
        if ( dx > 0) { Xinc = 1;}
        else { Xinc = -1;}
        double Y = Ys, Yinc = (double)dy/dx*Xinc;
        while( X != Xe)
        {
            X+=Xinc;
            Y+=Yinc;
            SetPixel(hdc, X, round(Y), colorref);
        }
    }
    else
    {
        int Y = Ys, Yinc;
        if (dy > 0){ Yinc = 1;}
        else { Yinc = -1;}
        double X = Xs;
        double Xinc = (double)dx/dy*Yinc;
        while (Y != Ye)
        {
            X+=Xinc;
            Y+=Yinc;
            SetPixel(hdc, round(X), Y, colorref);
        }
    }

}

void DrawMidpointline(HDC hdc, Point Begin, Point Finish, COLORREF colorref)
{
    int Xs, Ys, Xe, Ye;
    Xs = Begin.x;
    Ys = Begin.y;
    Xe = Finish.x;
    Ye = Finish.y;
    if (Xe < Xs)
    {
        Swap(Xe, Xs);
        Swap(Ye, Ys);
    }
    int deltaY = Ye - Ys;
    int deltaX = Xe - Xs;
    int x = Xs;
    int y = Ys;
    double m = (double)deltaY/deltaX;
    int d;
    SetPixel(hdc, x, y, colorref);
    if (m >= 0 && m <= 1)
    {
        while (x < Xe)
        {
            d = round( (y+0.5-Ys) * deltaX - (x+1-Xs) *deltaY);
            if (d>0){x++;}
            else {x++; y++;}
            SetPixel(hdc, x, y, colorref);
        }
    }
    else if (m > 1)
    {
        while (y < Ye)
        {
            d = round((y+1-Ys)*deltaX - (x+0.5-Xs)*deltaY);
            if (d<0){y++;}
            else {x++; y++;}
            SetPixel(hdc, x, y, colorref);
        }
    }
    else if (m < -1)
    {
        while (x < Xe)
        {
            d = round((y-1-Ys)*deltaX - (x+0.5-Xs) *deltaY);
            if (d>0){y--;}
            else {x++; y--;}
            SetPixel(hdc, x, y, colorref);
        }
    }
    else
    {
        while (x < Xe)
        {
            d = round((y-0.5-Ys)*deltaX - (x+1-Xs) * deltaY);
            if (d<0){x++;}
            else {x++; y--;}
            SetPixel(hdc, x, y, colorref);
        }
    }
}

///////////////////////////////////////////////////////

void Draw8points(HDC hdc, int xc, int yc, int a, int b, COLORREF colorref)
{
    SetPixel(hdc, xc+a, yc+b, colorref);
    SetPixel(hdc, xc-a, yc+b, colorref);
    SetPixel(hdc, xc-a, yc-b, colorref);
    SetPixel(hdc, xc+a, yc-b, colorref);
    SetPixel(hdc, xc+b, yc+a, colorref);
    SetPixel(hdc, xc-b, yc+a, colorref);
    SetPixel(hdc, xc-b, yc-a, colorref);
    SetPixel(hdc, xc+b, yc-a, colorref);
}

void circleDirect(HDC hdc, Point Begin, Point Finish, COLORREF colorref)
{
    int Xs, Ys, Xe, Ye;
    Xs = Begin.x;
    Ys = Begin.y;
    Xe = Finish.x;
    Ye = Finish.y;
    int x = 0, y = sqrt(pow((Xe- Xs), 2) + pow((Ye-Ys), 2));
    int r = y*y;
    Draw8points(hdc, Xs, Ys, x, y, colorref);
    while(x<y)
    {
        x++;
        y = round(sqrt((double)r-x*x));
        Draw8points(hdc, Xs, Ys, x, y, colorref);
    }
}

void CircleMidpoint(HDC hdc, Point Begin, Point Finish, COLORREF colorref)
{
    int Xs, Ys, Xe, Ye;
    Xs = Begin.x;
    Ys = Begin.y;
    Xe = Finish.x;
    Ye = Finish.y;
    int x = 0, y = sqrt(pow((Xe- Xs), 2) + pow((Ye-Ys), 2));
    int d = -y;
    Draw8points(hdc, Xs, Ys, x, y, colorref);
    while(x<y)
    {
        if(d<0){d+=2*x+3;}
        else
        {
            d+=2*(x-y)+5;
            y--;
        }
        x++;
        Draw8points(hdc, Xs, Ys, x, y, colorref);
    }
}

void CirclePolar(HDC hdc, Point Begin, Point Finish, COLORREF colorref)
{
    int Xs, Ys, Xe, Ye;
    Xs = Begin.x;
    Ys = Begin.y;
    Xe = Finish.x;
    Ye = Finish.y;
    int r = sqrt(pow((Xe- Xs), 2) + pow((Ye-Ys), 2));
    double x = r;
    double y = 0;
    double dtheta = 1.0/r;
    double cdtheta = cos(dtheta);
    double sdtheta = sin(dtheta);
    Draw8points(hdc, Xs, Ys, x, y, colorref);
    while (x > y)
    {
        double x1 = (x*cdtheta) - (y*sdtheta);
        y = (x*sdtheta) + (y*cdtheta);
        x = x1;
        Draw8points(hdc, Xs, Ys, x, y, colorref);
    }
}

///////////////////////////////////////////////////////

void NRFloodFill(HDC hdc, Point Begin, COLORREF Bordercolor, COLORREF FillColor)
{
    stack<int> stx;
    stack<int> sty;
    int px, py;
    stx.push(Begin.x);
    sty.push(Begin.y);
    while(!stx.empty() || !sty.empty())
    {
        px = stx.top();
        py = sty.top();
        stx.pop();
        sty.pop();
        COLORREF c = GetPixel(hdc, px, py);
        if(c==Bordercolor || c==FillColor) {continue;}
        SetPixel(hdc, px, py, FillColor);
        stx.push(px+1);sty.push(py);
        stx.push(px-1);sty.push(py);
        sty.push(py+1);stx.push(px);
        sty.push(py-1);stx.push(px);
    }
    cout<<"end"<<endl;
}

///////////////////////////////////////////////////////

union OutCode
{
	unsigned All : 4;
	struct { unsigned left : 1, right : 1, bottom : 1, top : 1; };
};

OutCode GetOutCode(double x, double y, int xleft, int xright, int ybottom, int ytop)
{
	OutCode result;
	result.All = 0;
	if (x < xleft) { result.left = 1;}
	else if (x > xright) { result.right = 1;}
	if (y > ybottom) { result.bottom = 1; }
	else if (y < ytop) { result.top = 1; }
	return result;
}

Point Vintersect(double xs, double ys, double xe, double ye, int xedge)
{
	Point temp;
	temp.x = xedge;
	temp.y = ((xedge - xs)*(ye - ys) / (double)(xe - xs)) + ys;
	return temp;
}

Point Hintersect(double xs, double ys, double xe, double ye, int yedge)
{
	Point temp;
	temp.x = yedge;
	temp.y = ((yedge - ys)*(xe - xs) / (double)(ye - ys)) + xs;
	return temp;
}

void LineClipping(HDC hdc, int xs, int ys, int xe, int ye, int xleft, int xright, int ytop, int ybottom, COLORREF colorref)
{

	OutCode out1 = GetOutCode(xs, ys, xleft, xright, ybottom, ytop);

	OutCode out2 = GetOutCode(xe, ye, xleft, xright, ybottom, ytop);

	while((out1.All || out2.All) && !(out1.All & out2.All))
	{
		if (out1.All)
        {

			if (out1.left)
            {
				Point v = Vintersect(xs, ys, xe, ye, xleft);
				xs = v.x;
				ys = v.y;
			}
			else if (out1.right)
			{
				Point v = Vintersect(xs, ys, xe, ye, xright);
				xs = v.x;
				ys = v.y;
			}
			else if (out1.bottom)
			{
				Point v = Hintersect(xs, ys, xe, ye, ybottom);
				xs = v.x;
				ys = v.y;
			}
			else if (out1.top)
			{
				Point v = Hintersect(xs, ys, xe, ye, ytop);
				xs = v.x;
				ys = v.y;
			}

			out1 = GetOutCode(xs, ys, xleft, xright, ybottom, ytop);
		}
		else if (out2.All)
        {
			if (out2.left)
			{
				Point v = Vintersect(xs, ys, xe, ye, xleft);
				xe = v.x;
				ye = v.y;
			}
			else if (out2.right)
			{
				Point v = Vintersect(xs, ys, xe, ye, xright);
				xe = v.x;
				ye = v.y;
			}
			else if (out2.bottom)
			{
				Point v = Hintersect(xs, ys, xe, ye, ybottom);
				xe = v.x;
				ye = v.y;
			}
			else if (out2.top)
			{
				Point v = Hintersect(xs, ys, xe, ye, ytop);
				xe = v.x;
				ye = v.y;
			}

			out2 = GetOutCode(xe, ye, xleft, xright, ybottom, ytop);
		}
	}
    if (out1.All == 0 && out2.All == 0)
    {
        Point startpoint;

        startpoint.x=xs;
        startpoint.y=ys;

        Point endpoint;

        endpoint.x=xe;
        endpoint.y=ye;

        ClippedLines.push_back(startpoint);
        ClippedLines.push_back(endpoint);

		DrawDDALine(hdc,startpoint,endpoint, colorref);
	}
}

void CohenSuth(HDC hdc, int xleft, int xright, int ytop, int ybottom, COLORREF colorref)
{
    COLORREF c = RGB(255, 0, 0);
    for(int i = 0; i < Lines.size()-1; i+=2)
    {
        cout << Lines[i].x << " " << Lines[i].y << endl;
        cout << Lines[i+1].x << " " << Lines[i+1].y << endl;
        LineClipping(hdc, Lines[i].x, Lines[i].y, Lines[i+1].x, Lines[i+1].y, xleft, xright, ytop, ybottom, c);
    }
    int LineSize = Lines.size();
    Lines.clear();
    for(int i = 0; i < LineSize; i++)
    {
        Lines.push_back(ClippedLines[i]);
    }
}



////////////////////////////////////////////////////////

void HermitDraw(HDC hdc, Point one, Point two, Point three, Point four, COLORREF colorref)
{
    //cout<<"hermit function"<<endl;
    int x = one.x, y = one.y;
    SetPixel(hdc, round(x), round(y), colorref);

    int alpha0 = one.x;
    int beta0 = one.y;

    int alpha1 = two.x;
    int beta1 = two.y;

    int alpha2 = (-3 * one.x) - (2 * two.x) - (-3 * three.x) - four.x;
    int beta2 = (-3 * one.y) - (2 * two.y) - (-3 * three.y) - four.y;

    int alpha3 = (2 * one.x) + two.x + (-2 * three.x) + four.x;
    int beta3 = (2 * one.y) + (two.y) + (-2 * three.y) + (four.y);

    for (double t = 0; t <= 1; t+=0.0001)
    {
        x = alpha0 + (alpha1 * t) + (alpha2 * (t * t)) + (alpha3 * (t * t * t));
        y = beta0 +  (beta1 * t) + (beta2 * (t * t)) + (beta3 * (t * t * t));
        SetPixel(hdc, round(x), round(y), colorref);
        //cout<<"for loop"<<endl;
    }
}

void BezierDraw(HDC hdc, Point one, Point two, Point three, Point four, COLORREF colorref)
{
    double x = one.x, y = one.y;
    SetPixel(hdc, round(x), round(y), colorref);
    for (double t = 0; t <= 1; t += 0.0001)
	{
		x = ((1 - t)*(1 - t)*(1 - t)*one.x) + (3 * (1 - t)*(1 - t)*t*two.x) + (3 * (1 - t)*t*t*three.x) + (t*t*t*four.x);
		y = ((1 - t)*(1 - t)*(1 - t)*one.y) + (3 * (1 - t)*(1 - t)*t*two.y) + (3 * (1 - t)*t*t*three.y) + (t*t*t*four.y);
		SetPixel(hdc, round(x), round(y), colorref);
	}
}

///////////////////////////////////////////////////////

void DrawPolygon(HDC hdc, Point one, Point two, Point three, Point four, COLORREF colorref)
{
    DrawDDALine(hdc, one, two, colorref);
    DrawDDALine(hdc, two, three, colorref);
    DrawDDALine(hdc, three, four, colorref);
    DrawDDALine(hdc, four, one, colorref);
}

///////////////////////////////////////////////////////
void SaveToFile()
{
    ofstream Linefile;
    ofstream Circlefile;
    ofstream Fillfile;
    ofstream Curvefile;

    Linefile.open("LineFile.txt");
    Circlefile.open("CircleFile.txt");
    Fillfile.open("FillFile.txt");
    Curvefile.open("CurveFile.txt");


    for(int i = 0; i< Lines.size(); i++)
    {
        Linefile << Lines[i].x << " " << Lines[i].y << endl;
    }

    for(int i = 0; i < Circle.size(); i++)
    {
        Circlefile << Circle[i].x << " " << Circle[i].y << endl;
    }

    for (int i = 0; i < Fill.size(); i++)
    {
        Fillfile << Fill[i].x << " " << Fill[i].y << endl;
    }

    for(int i = 0; i < Curves.size(); i++)
    {
        Curvefile << Curves[i].x << " " << Curves[i].y << endl;
    }

    Linefile.close();
    Circlefile.close();
    Fillfile.close();
    Curvefile.close();
}

void LoadfromFile()
{
    ifstream LineFile("LineFile.txt");
    ifstream CircleFile("CircleFile.txt");
    ifstream FillFile("FillFile.txt");
    ifstream CurveFile("CurveFile.txt");

    //LineFile.open("LineFile.txt");

    Point temp;
    int x, y;

    cout << "hello" << endl;
    if(LineFile.is_open())
    {
        LineFile.clear();
        while(!LineFile.eof())
        {
            LineFile >> x;
            if (LineFile.eof()){break;}
            LineFile >> y;
            if (LineFile.eof()){break;}
            temp.x = x;
            temp.y = y;
            LoadLines.push_back(temp);
            cout << temp.x << " " << temp.y << endl;
        }
    }
    if(CircleFile.is_open())
    {
        CircleFile.clear();
        while(!CircleFile.eof())
        {
            CircleFile >> x;
            if (CircleFile.eof()){break;}
            CircleFile >> y;
            if(CircleFile.eof()){break;}
            temp.x = x;
            temp.y = y;
            LoadCircles.push_back(temp);
        }
    }
    if(FillFile.is_open())
    {
        FillFile.clear();
        while(!FillFile.eof())
        {
            FillFile >> x;
            if(FillFile.eof()){break;}
            FillFile >> y;
            if(FillFile.eof()){break;}
            temp.x = x;
            temp.y = y;
            LoadFill.push_back(temp);
        }
    }
    if(CurveFile.is_open())
    {
        CurveFile.clear();
        while(!CurveFile.eof())
        {
            CurveFile >> x;
            if(CurveFile.eof()){break;}
            CurveFile >> y;
            if(CurveFile.eof()){break;}
            temp.x = x;
            temp.y = y;
            LoadCurve.push_back(temp);
        }
    }

    LineFile.close();
    CircleFile.close();
    FillFile.close();
    CurveFile.close();
}

void DrawAll(HDC hdc, COLORREF colorref)
{
    COLORREF fillcolor = RGB(0, 0, 255);
    for (int i = 0; i < LoadLines.size(); i+=2)
    {
        DrawDDALine(hdc, LoadLines[i], LoadLines[i+1], colorref);
    }
    for (int i = 0; i < LoadCircles.size(); i+=2)
    {
        CircleMidpoint(hdc, LoadCircles[i], LoadCircles[i+1], colorref);
    }
    for (int i = 0; i < LoadFill.size(); i++)
    {
        NRFloodFill(hdc, LoadFill[i], colorref, fillcolor);
    }
    for (int i = 0; i < LoadCurve.size(); i+=4)
    {
        BezierDraw(hdc, LoadCurve[i], LoadCurve[i+1], LoadCurve[i+2], LoadCurve[i+3], colorref);
    }
}

//////////////////////////////////////////////////////
int WINAPI WinMain (HINSTANCE hThisInstance,
                     HINSTANCE hPrevInstance,
                     LPSTR lpszArgument,
                     int nCmdShow)
{
    HWND hwnd;               /* This is the handle for our window */
    MSG messages;            /* Here messages to the application are saved */
    WNDCLASSEX wincl;        /* Data structure for the windowclass */

    /* The Window structure */
    wincl.hInstance = hThisInstance;
    wincl.lpszClassName = szClassName;
    wincl.lpfnWndProc = WindowProcedure;      /* This function is called by windows */
    wincl.style = CS_DBLCLKS;                 /* Catch double-clicks */
    wincl.cbSize = sizeof (WNDCLASSEX);

    /* Use default icon and mouse-pointer */
    wincl.hIcon = LoadIcon (NULL, IDI_APPLICATION);
    wincl.hIconSm = LoadIcon (NULL, IDI_APPLICATION);
    wincl.hCursor = LoadCursor (NULL, IDC_ARROW);
    wincl.lpszMenuName = MAKEINTRESOURCE(1);
    wincl.cbClsExtra = 0;                      /* No extra bytes after the window class */
    wincl.cbWndExtra = 0;                      /* structure or the window instance */
    /* Use Windows's default colour as the background of the window */
    wincl.hbrBackground = (HBRUSH) COLOR_BACKGROUND;

    /* Register the window class, and if it fails quit the program */
    if (!RegisterClassEx (&wincl))
        return 0;

    /* The class is registered, let's create the program*/
    hwnd = CreateWindowEx (
           0,                   /* Extended possibilites for variation */
           szClassName,         /* Classname */
           _T("Code::Blocks Template Windows App"),       /* Title Text */
           WS_OVERLAPPEDWINDOW, /* default window */
           CW_USEDEFAULT,       /* Windows decides the position */
           CW_USEDEFAULT,       /* where the window ends up on the screen */
           544,                 /* The programs width */
           375,                 /* and height in pixels */
           HWND_DESKTOP,        /* The window is a child-window to desktop */
           NULL,                /* No menu */
           hThisInstance,       /* Program Instance handler */
           NULL                 /* No Window Creation data */
           );

    /* Make the window visible on the screen */
    ShowWindow (hwnd, nCmdShow);

    /* Run the message loop. It will run until GetMessage() returns 0 */
    while (GetMessage (&messages, NULL, 0, 0))
    {
        /* Translate virtual-key messages into character messages */
        TranslateMessage(&messages);
        /* Send message to WindowProcedure */
        DispatchMessage(&messages);
    }

    /* The program return-value is 0 - The value that PostQuitMessage() gave */
    return messages.wParam;
}



/*  This function is called by the Windows function DispatchMessage()  */

LRESULT CALLBACK WindowProcedure (HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    PAINTSTRUCT p;
    COLORREF coloref = RGB(0,0,0);
    HDC hdc;
    switch (message)                  /* handle the messages */
    {
        case WM_PAINT:
            if (StraightLine && firstpoint && secondpoint)
            {
                hdc = BeginPaint(hwnd, &p);
                Lines.push_back(First);
                Lines.push_back(Second);
                DrawSimpleLine(hdc, First, Second, coloref);
                firstpoint = false;
                secondpoint = false;
                InvalidateRect(hwnd, NULL, FALSE);
                EndPaint(hwnd, &p);
            }
            if (DDALine && firstpoint && secondpoint)
            {
                hdc = BeginPaint(hwnd, &p);
                Lines.push_back(First);
                Lines.push_back(Second);
                DrawDDALine(hdc, First, Second, coloref);
                firstpoint = false;
                secondpoint = false;
                InvalidateRect(hwnd, NULL, FALSE);
                EndPaint(hwnd, &p);
            }
            if (MidpointLine && firstpoint && secondpoint)
            {
                hdc = BeginPaint(hwnd, &p);
                Lines.push_back(First);
                Lines.push_back(Second);
                DrawMidpointline(hdc, First, Second, coloref);
                firstpoint = false;
                secondpoint = false;
                InvalidateRect(hwnd, NULL, FALSE);
                EndPaint(hwnd, &p);
            }
            if (DirectCircle && firstpoint && secondpoint)
            {
                hdc = BeginPaint(hwnd, &p);
                Circle.push_back(First);
                Circle.push_back(Second);
                circleDirect(hdc, First, Second, coloref);
                firstpoint = false;
                secondpoint = false;
                InvalidateRect(hwnd, NULL, FALSE);
                EndPaint(hwnd, &p);
            }
            if(MidpointCircle && firstpoint && secondpoint)
            {
                hdc = BeginPaint(hwnd, &p);
                Circle.push_back(First);
                Circle.push_back(Second);
                CircleMidpoint(hdc, First, Second, coloref);
                firstpoint = false;
                secondpoint = false;
                InvalidateRect(hwnd, NULL, FALSE);
                EndPaint(hwnd, &p);

            }
            if (PolarCircle && firstpoint && secondpoint)
            {
                hdc = BeginPaint(hwnd, &p);
                Circle.push_back(First);
                Circle.push_back(Second);
                CirclePolar(hdc, First, Second, coloref);
                firstpoint = false;
                secondpoint = false;
                InvalidateRect(hwnd, NULL, FALSE);
                EndPaint(hwnd, &p);
            }
            if (ClipLine && ClipPoint1 && ClipPoint2)
            {
                cout << "Entered if" << endl;
                hdc = BeginPaint(hwnd, &p);
                Point temp1; temp1.x = Xleft; temp1.y = Ytop;
                Point temp2; temp2.x = Xright; temp2.y = Ybottom;
                Point temp3; temp3.x = Xright; temp3.y = Ytop;
                Point temp4; temp4.x = Xleft; temp4.y = Ybottom;
                DrawPolygon(hdc, temp1, temp3, temp2, temp4, coloref);
                CohenSuth(hdc, Xleft, Xright, Ytop, Ybottom, coloref);
                firstpoint = false;
                secondpoint = false;
                ClipPoint1 = false;
                ClipPoint2 = false;
                InvalidateRect(hwnd, NULL, FALSE);
                EndPaint(hwnd, &p);
            }
            if (SetFill && firstpoint)
            {
                hdc = BeginPaint(hwnd, &p);
                Fill.push_back(First);
                COLORREF fillcolor = RGB(0, 0, 255);
                NRFloodFill(hdc, First, coloref, fillcolor);
                firstpoint = false;
                InvalidateRect(hwnd, NULL, FALSE);
                EndPaint(hwnd, &p);
            }
            if (Hermit && firstpoint && secondpoint && thirdpoint && fourthpoint)
            {
                hdc = BeginPaint(hwnd, &p);
                Curves.push_back(First);
                Curves.push_back(Second);
                Curves.push_back(Third);
                Curves.push_back(Fourth);
                HermitDraw(hdc, First, Second, Third, Fourth, coloref);
                firstpoint = false;
                secondpoint = false;
                thirdpoint = false;
                fourthpoint = false;
                InvalidateRect(hwnd, NULL, FALSE);
                EndPaint(hwnd, &p);
            }
            if (Bezier && firstpoint && secondpoint && thirdpoint && fourthpoint)
            {
                hdc = BeginPaint(hwnd, &p);
                Curves.push_back(First);
                Curves.push_back(Second);
                Curves.push_back(Third);
                Curves.push_back(Fourth);
                BezierDraw(hdc, First, Second, Third, Fourth, coloref);
                firstpoint = false;
                secondpoint = false;
                thirdpoint = false;
                fourthpoint = false;
                InvalidateRect(hwnd, NULL, FALSE);
                EndPaint(hwnd, &p);
            }
            if(load)
            {
                hdc = BeginPaint(hwnd, &p);
                LoadfromFile();
                cout << "after load"<<endl;
                load = false;
                DrawAll(hdc, coloref);
                InvalidateRect(hwnd, NULL, FALSE);
                EndPaint(hwnd, &p);
            }
            if (polygon &&  firstpoint && secondpoint && thirdpoint && fourthpoint)
            {
                hdc = BeginPaint(hwnd, &p);
                DrawPolygon(hdc, First, Second, Third, Fourth, coloref);
                firstpoint = false;
                secondpoint = false;
                thirdpoint = false;
                fourthpoint = false;
                InvalidateRect(hwnd, NULL, FALSE);
                EndPaint(hwnd, &p);
            }
        break;
        case WM_DESTROY:
            PostQuitMessage (0);       /* send a WM_QUIT to the message queue */
            break;
        case WM_LBUTTONDOWN:
            if (!Hermit || !Bezier)
            {
                if (ClipLine)
                {
                    if (ClipPoint1 == false)
                    {
                        Xleft = LOWORD(lParam);
                        Ytop  = HIWORD(lParam);
                        ClipPoint1 = true;
                        cout << "Clipping Xleft = " << Xleft << " Ytop = " << Ytop << endl;
                    }
                    else if (ClipPoint1 && ClipPoint2 == false)
                    {
                        Xright = LOWORD(lParam);
                        Ybottom= HIWORD(lParam);
                        ClipPoint2 = true;
                        cout << "Clipping Xright = " << Xright << " Ybottom = " << Ybottom << endl;
                    }
                }
                else
                {
                   if(!firstpoint)
                    {
                        First.x = LOWORD(lParam);
                        First.y = HIWORD(lParam);
                        firstpoint = true;
                    }
                    else if (firstpoint && secondpoint == false)
                    {
                        Second.x = LOWORD(lParam);
                        Second.y = HIWORD(lParam);
                        secondpoint = true;
                    }
                    else if (firstpoint && secondpoint && thirdpoint == false)
              {
                   Third.x = LOWORD(lParam);
                   Third.y = HIWORD(lParam);
                    thirdpoint = true;
                    cout<<Third.x<<" "<<Third.y<<endl;
                }
                else if (firstpoint && secondpoint && thirdpoint && fourthpoint == false)
                {
                    Fourth.x = LOWORD(lParam);
                    Fourth.y = HIWORD(lParam);
                    fourthpoint = true;
                    cout << Fourth.x <<" "<<Fourth.y<<endl;
                }
                }
            }
            else
            {
                if(!firstpoint)
                {
                    First.x = LOWORD(lParam);
                    First.y = HIWORD(lParam);
                    cout<< First.x <<" "<<First.y<<endl;
                    firstpoint = true;
                }
                else if (firstpoint && secondpoint == false)
                {
                    Second.x = LOWORD(lParam);
                    Second.y = HIWORD(lParam);
                    secondpoint = true;
                    cout<<Second.x<< " "<<Second.y<<endl;
                }
            }

//            if(!firstpoint)
//                {
//                    First.x = LOWORD(lParam);
//                    First.y = HIWORD(lParam);
//                    cout<< First.x <<" "<<First.y<<endl;
//                    firstpoint = true;
//                }
//                else if (firstpoint && secondpoint == false)
//                {
//                    Second.x = LOWORD(lParam);
//                    Second.y = HIWORD(lParam);
//                    secondpoint = true;
//                    cout<<Second.x<< " "<<Second.y<<endl;
//                }
//                else if (firstpoint && secondpoint && thirdpoint == false)
//                {
//                    Third.x = LOWORD(lParam);
//                    Third.y = HIWORD(lParam);
//                    thirdpoint = true;
//                    cout<<Third.x<<" "<<Third.y<<endl;
//                }
//                else if (firstpoint && secondpoint && thirdpoint && fourthpoint == false)
//                {
//                    Fourth.x = LOWORD(lParam);
//                    Fourth.y = HIWORD(lParam);
//                    fourthpoint = true;
//                    cout << Fourth.x <<" "<<Fourth.y<<endl;
//                }
            break;
        case WM_COMMAND:
            menuItemID = LOWORD(wParam);
            switch(menuItemID)
            {
                case MY_FILE_EXIT_ID:
                    PostQuitMessage(0);
                break;
                case MY_LINE_DIRECT_ID:
                    StraightLine = true;
                    DDALine = false;
                    MidpointLine = false;
                    MidpointCircle = false;
                    DirectCircle =false;
                    PolarCircle = false;
                    SetFill = false;
                    ClipLine = false;
                    ClipPolygon = false;
                    Bezier = false;
                    Hermit = false;
                    load = false;
                    polygon = false;
                break;
                case MY_LINE_DDA_ID:
                    StraightLine = false;
                    DDALine = true;
                    MidpointLine = false;
                    MidpointCircle = false;
                    DirectCircle =false;
                    PolarCircle = false;
                    SetFill = false;
                    ClipLine = false;
                    ClipPolygon = false;
                    Bezier = false;
                    Hermit = false;
                    load = false;
                    polygon = false;
                break;
                case MY_LINE_MIDPOINT_ID:
                    StraightLine = false;
                    DDALine = false;
                    MidpointLine = true;
                    MidpointCircle = false;
                    DirectCircle =false;
                    PolarCircle = false;
                    SetFill = false;
                    ClipLine = false;
                    ClipPolygon = false;
                    Bezier = false;
                    Hermit = false;
                    load = false;
                    polygon = false;
                break;
                case MY_DirectCircle_MENU_ID:
                    StraightLine = false;
                    DDALine = false;
                    MidpointLine = false;
                    MidpointCircle = false;
                    DirectCircle = true;
                    PolarCircle = false;
                    SetFill = false;
                    ClipLine = false;
                    ClipPolygon = false;
                    Bezier = false;
                    Hermit = false;
                    load = false;
                    polygon = false;
                break;
                case MY_MIDPOINTCIRCLE_MENU_ID:
                    StraightLine = false;
                    DDALine = false;
                    MidpointLine = false;
                    MidpointCircle = true;
                    DirectCircle =false;
                    PolarCircle = false;
                    SetFill = false;
                    ClipLine = false;
                    ClipPolygon = false;
                    Bezier = false;
                    Hermit = false;
                    load = false;
                    polygon = false;
                break;
                case MY_POLARCIRCLE_MENU_ID:
                    StraightLine = false;
                    DDALine = false;
                    MidpointLine = false;
                    MidpointCircle = false;
                    DirectCircle =false;
                    PolarCircle = true;
                    SetFill = false;
                    ClipLine = false;
                    ClipPolygon = false;
                    Bezier = false;
                    Hermit = false;
                    load = false;
                    polygon = false;
                break;
                case MY_FLOOD_FILLING_MENU_ID:
                    StraightLine = false;
                    DDALine = false;
                    MidpointLine = false;
                    MidpointCircle = false;
                    DirectCircle =false;
                    PolarCircle = false;
                    SetFill = true;
                    ClipLine = false;
                    ClipPolygon = false;
                    Bezier = false;
                    Hermit = false;
                    load = false;
                    polygon = false;
                break;
                case MY_CLIP_LINE_MENU_ID:
                    StraightLine = false;
                    DDALine = false;
                    MidpointLine = false;
                    MidpointCircle = false;
                    DirectCircle =false;
                    PolarCircle = false;
                    SetFill = false;
                    ClipLine = true;
                    ClipPolygon = false;
                    Bezier = false;
                    Hermit = false;
                    load = false;
                    polygon = false;
                break;
                case MY_HERMIT_MENU_ID:
                    StraightLine = false;
                    DDALine = false;
                    MidpointLine = false;
                    MidpointCircle = false;
                    DirectCircle =false;
                    PolarCircle = false;
                    SetFill = false;
                    ClipLine = false;
                    ClipPolygon = false;
                    Bezier = false;
                    Hermit = true;
                    load = false;
                    polygon = false;
                break;
                case MY_BEZIER_MENU_ID:
                    StraightLine = false;
                    DDALine = false;
                    MidpointLine = false;
                    MidpointCircle = false;
                    DirectCircle =false;
                    PolarCircle = false;
                    SetFill = false;
                    ClipLine = false;
                    ClipPolygon = false;
                    Bezier = true;
                    Hermit = false;
                    load = false;
                    polygon = false;
                break;
                case MY_SAVE_MENU_ID:
                    SaveToFile();
                    cout<<"save"<<endl;
                    load = false;
                break;
                case MY_LOAD_MENU_ID:
                    cout<<"load true/n";
                    StraightLine = false;
                    DDALine = false;
                    MidpointLine = false;
                    MidpointCircle = false;
                    DirectCircle =false;
                    PolarCircle = false;
                    SetFill = false;
                    ClipLine = false;
                    ClipPolygon = false;
                    Bezier = false;
                    Hermit = false;
                    load = true;
                    polygon = false;
                    break;
                case MY_POLYGON_MENU_ID:
                    StraightLine = false;
                    DDALine = false;
                    MidpointLine = false;
                    MidpointCircle = false;
                    DirectCircle =false;
                    PolarCircle = false;
                    SetFill = false;
                    ClipLine = false;
                    ClipPolygon = false;
                    Bezier = false;
                    Hermit = false;
                    load = false;
                    polygon = true;
                break;
                default:
                    return DefWindowProc(hwnd, message, wParam, lParam);
            }

        default:                      /* for messages that we don't deal with */
            return DefWindowProc (hwnd, message, wParam, lParam);
    }

    return 0;
}
