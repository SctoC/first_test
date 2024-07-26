
// Mfc_ScreenShotDlg.cpp: 实现文件
//

#include "pch.h"
#include "framework.h"
#include "Mfc_ScreenShot.h"
#include "Mfc_ScreenShotDlg.h"
#include "afxdialogex.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif


// 用于应用程序“关于”菜单项的 CAboutDlg 对话框

class CAboutDlg : public CDialogEx
{
public:
	CAboutDlg();

// 对话框数据
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_ABOUTBOX };
#endif

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 支持

// 实现
protected:
	DECLARE_MESSAGE_MAP()
};

CAboutDlg::CAboutDlg() : CDialogEx(IDD_ABOUTBOX)
{
}

void CAboutDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
}

BEGIN_MESSAGE_MAP(CAboutDlg, CDialogEx)
END_MESSAGE_MAP()


// CMfcScreenShotDlg 对话框



CMfcScreenShotDlg::CMfcScreenShotDlg(CWnd* pParent /*=nullptr*/)
	: CDialogEx(IDD_MFC_SCREENSHOT_DIALOG, pParent)
{
	/*m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);*/
}

void CMfcScreenShotDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
}

BEGIN_MESSAGE_MAP(CMfcScreenShotDlg, CDialogEx)
	ON_WM_SYSCOMMAND()
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	ON_WM_MOUSEMOVE()
	ON_WM_LBUTTONDOWN()
	ON_WM_LBUTTONUP()
	ON_WM_RBUTTONDBLCLK()
	ON_WM_RBUTTONUP()
END_MESSAGE_MAP()


// CMfcScreenShotDlg 消息处理程序

BOOL CMfcScreenShotDlg::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	// 使窗口铺满屏幕
	CRect screenRect;
	::SystemParametersInfo(SPI_GETWORKAREA, 0, &screenRect, 0);
	MoveWindow(&screenRect);

	// 设置窗口样式，去掉图标
	LONG style = GetWindowLong(GetSafeHwnd(), GWL_STYLE);
	style &= ~(WS_CAPTION | WS_THICKFRAME); // 去掉标题栏和边框
	SetWindowLong(GetSafeHwnd(), GWL_STYLE, style);
	SetWindowPos(&wndTopMost, 0, 0, screenRect.Width(), screenRect.Height(), SWP_NOACTIVATE | SWP_NOREDRAW | SWP_NOMOVE);

	//设置允许透明
	style |= WS_EX_LAYERED;
	SetWindowLong(m_hWnd, GWL_EXSTYLE, style);

	// 设置透明度
	// 这里设置背景色为黑色 (RGB(0, 0, 0)) 作为透明色，透明度为 200 (0-255)
	SetLayeredWindowAttributes(RGB(0, 0, 0), 100, LWA_COLORKEY | LWA_ALPHA);

	return TRUE; 
}

void CMfcScreenShotDlg::OnSysCommand(UINT nID, LPARAM lParam)
{
	if ((nID & 0xFFF0) == IDM_ABOUTBOX)
	{
		CAboutDlg dlgAbout;
		dlgAbout.DoModal();
	}
	else
	{
		CDialogEx::OnSysCommand(nID, lParam);
	}
}



void CMfcScreenShotDlg::OnPaint()
{
	    //设备上下文，用来绘图的。至于为什么叫设备上下文，咱也不知道，起名字挺迷的
		CPaintDC dc(this); 

		//如果开始区域选择
		if (m_bCapturing)
		{
			//根据我们记录的起始坐标 和 结束坐标 新建一个矩形
			CRect rect(m_ptStart, m_ptEnd);

			//刷子是用来填充的，这里不做填充
			CBrush brush;
			brush.CreateStockObject(NULL_BRUSH); // 不填充矩形
			CBrush* pOldBrush = dc.SelectObject(&brush);

			//笔是用来绘制边框的
			CPen pen(PS_SOLID, 1, RGB(255, 0, 0)); // 红色画笔
			CPen* pOldPen = dc.SelectObject(&pen);

			//绘制矩形
			dc.Rectangle(&rect);

			//还原之前的刷子和笔
			dc.SelectObject(pOldPen);
			dc.SelectObject(pOldBrush);
		}

		CDialogEx::OnPaint();
}

//当用户拖动最小化窗口时系统调用此函数取得光标
//显示。
HCURSOR CMfcScreenShotDlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}



void CMfcScreenShotDlg::OnMouseMove(UINT nFlags, CPoint point)
{
	//如果开始了区域选择
	if (m_bCapturing)
	{
		//记录当前鼠标坐标为矩形区域的结束坐标
		m_ptEnd = point;
		//触发重绘，调用OnPaint()
		Invalidate();
	}
	CDialogEx::OnMouseMove(nFlags, point);
}


void CMfcScreenShotDlg::OnLButtonDown(UINT nFlags, CPoint point)
{
	//开始选择区域
	m_bCapturing = TRUE;
	//记录当前鼠标坐标为选择区域的起始坐标
	m_ptStart = point;

	CDialogEx::OnLButtonDown(nFlags, point);
}


void CMfcScreenShotDlg::OnLButtonUp(UINT nFlags, CPoint point)
{
	//如果开始了区域选择
	if (m_bCapturing)
	{
		//停止区域选择
		m_bCapturing = FALSE;
		//记录当前鼠标坐标为矩形区域的结束坐标
		m_ptEnd = point;

		// 开始截图保存，根据我们记录的矩形起始坐标和结束坐标。
		CaptureScreen();
	}
	CDialogEx::OnLButtonUp(nFlags, point);
}

//主打的就是繁琐
void CMfcScreenShotDlg::CaptureScreen()
{
	// 获取屏幕的设备上下文  
	HDC hScreenDC = :: GetDC(NULL);
	// 创建内存上下文，用于将所选区域绘制到新建的位图中
	HDC hMemoryDC = CreateCompatibleDC(hScreenDC);

	// 计算矩形区域的宽度和高度  
	int width = abs(m_ptEnd.x - m_ptStart.x);
	int height = abs(m_ptEnd.y - m_ptStart.y);

	// 创建一个与屏幕设备上下文兼容的位图  
	HBITMAP hBitmap = CreateCompatibleBitmap(hScreenDC, width, height);
	//将位图给到内存上下文
	HBITMAP hOldBitmap = (HBITMAP)SelectObject(hMemoryDC, hBitmap);

	// 将所选区域绘制到新建的位图中
	BitBlt(hMemoryDC, 0, 0, width, height, hScreenDC, min(m_ptStart.x, m_ptEnd.x), min(m_ptStart.y, m_ptEnd.y), SRCCOPY);
	hBitmap = (HBITMAP)SelectObject(hMemoryDC, hOldBitmap);

	// 位图的信息头
	BITMAPINFOHEADER bi;
	bi.biSize = sizeof(BITMAPINFOHEADER);
	bi.biWidth = width;
	bi.biHeight = -height;  // top-down DIB  
	bi.biPlanes = 1;
	bi.biBitCount = 24;
	bi.biCompression = BI_RGB;
	bi.biSizeImage = 0;
	bi.biXPelsPerMeter = 0;
	bi.biYPelsPerMeter = 0;
	bi.biClrUsed = 0;
	bi.biClrImportant = 0;

	//位图的大小（字节）
	DWORD dwBmpSize = ((width * bi.biBitCount + 31) / 32) * 4 * height;

	//新建内存，用来存位图
	HANDLE hDIB = GlobalAlloc(GHND, dwBmpSize);
	char* lpbitmap = (char*)GlobalLock(hDIB);

	//将位图存入内存
	GetDIBits(hScreenDC, hBitmap, 0, (UINT)height, lpbitmap, (BITMAPINFO*)&bi, DIB_RGB_COLORS);

	// 将内存中的位图数据，存入文件
	CFile file;
	if (file.Open(_T("screenshot.bmp"), CFile::modeCreate | CFile::modeWrite))
	{
		//位图的文件头
		BITMAPFILEHEADER bfh;
		bfh.bfOffBits = sizeof(BITMAPFILEHEADER) + sizeof(BITMAPINFOHEADER);
		bfh.bfSize = dwBmpSize + sizeof(BITMAPFILEHEADER) + sizeof(BITMAPINFOHEADER);
		bfh.bfType = 0x4D42; // BM  
		//写入文件头
		file.Write(&bfh, sizeof(BITMAPFILEHEADER));
		//写入信息头
		file.Write(&bi, sizeof(BITMAPINFOHEADER));
		//写入位图
		file.Write(lpbitmap, dwBmpSize);
		file.Close();
	}

	// 清理资源
	GlobalUnlock(hDIB);
	GlobalFree(hDIB);
	DeleteObject(hBitmap);
	DeleteDC(hMemoryDC);
	::ReleaseDC(NULL, hScreenDC);
}


void CMfcScreenShotDlg::OnRButtonDblClk(UINT nFlags, CPoint point)
{
	// TODO: 在此添加消息处理程序代码和/或调用默认值
	EndDialog(IDCANCEL);

	// 终止程序
	PostQuitMessage(0);
	CDialogEx::OnRButtonDblClk(nFlags, point);
}


void CMfcScreenShotDlg::OnRButtonUp(UINT nFlags, CPoint point)
{

	EndDialog(IDCANCEL);

	// 终止程序
	PostQuitMessage(0);
	CDialogEx::OnRButtonUp(nFlags, point);
}
