/************************************************************************/
/* File Name   : app.cpp                                                */
/* Creator     : ax.minaduki@gmail.com                                  */
/* Create Time : Nov 27th, 2008                                         */
/* Module      : Common library                                         */
/* Descript    : DApp class implementation                              */
/************************************************************************/

#include <app.hpp>

/************************************************************************/

#ifdef _WIN32
static DApp	*s_AppInstance = NULL;
#endif

/************************************************************************/

#ifdef _WIN32

static LRESULT CALLBACK WindowProc(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam)
{
//	HDC hdc;
//	PAINTSTRUCT ps;
//	RECT rect;
	POINT pos;

	switch (msg) {
//	case WM_CREATE:
//		PlaySound(TEXT("hellowin.wav"), NULL, SND_FILENAME | SND_ASYNC);
//		return 0;
//	case WM_PAINT:
//		hdc = BeginPaint(hwnd, &ps);
//		return 0;
	case WM_LBUTTONDOWN:
		pos.x = LOWORD(lparam);
		pos.y = HIWORD(lparam);
		s_AppInstance->OnLButtonDown(pos);
		break;
	case WM_LBUTTONUP:
		pos.x = LOWORD(lparam);
		pos.y = HIWORD(lparam);
		s_AppInstance->OnLButtonUp(pos);
		break;
	case WM_RBUTTONDOWN:
		pos.x = LOWORD(lparam);
		pos.y = HIWORD(lparam);
		s_AppInstance->OnRButtonDown(pos);
		break;
	case WM_RBUTTONUP:
		pos.x = LOWORD(lparam);
		pos.y = HIWORD(lparam);
		s_AppInstance->OnRButtonUp(pos);
		break;
	case WM_CHAR:
		s_AppInstance->OnChar(wparam);
		break;
	case WM_SYSCHAR:
		if (wparam == VK_RETURN && lparam & 0x20000000) {
			s_AppInstance->OnAltEnter();
			return 0;
		}
		break;
	case WM_DESTROY:
		::PostQuitMessage(0);
		return 0;
	}

	return ::DefWindowProc(hwnd, msg, wparam, lparam);
}

#endif

/************************************************************************/

DApp::DApp() :
#ifdef _WIN32
	m_Wnd(NULL),
#endif
	m_RetCode(0)
{
	m_Core.name = NULL;
	m_Pos.x = 0;
	m_Pos.y = 0;
	m_Size.cx = 800;
	m_Size.cy = 600;
#ifdef _WIN32
	m_Core.hinst = NULL;
	m_Core.cmd = NULL;
	m_Core.show = 0;
	DVarClr(m_Freq);
	DVarClr(m_StartTime);
#else
	m_Core.argc = 0;
	m_Core.argv = NULL;
#endif
	s_AppInstance = this;
}

DApp::~DApp()
{
	s_AppInstance = NULL;
}

BOOL DApp::Initialize(VOID)
{
	if (!Prepare(m_Core, m_Pos, m_Size))
		return FALSE;

	if (!Check())
		return FALSE;

#ifdef _WIN32

	if (!::QueryPerformanceFrequency(&m_Freq))
		return FALSE;

	if (!::QueryPerformanceCounter(&m_StartTime))
		return FALSE;

	WNDCLASS wnd_class;

	if (!::GetClassInfo(m_Core.hinst, m_Core.name, &wnd_class)) {

 		wnd_class.style = CS_HREDRAW | CS_VREDRAW;
		wnd_class.lpfnWndProc = ::WindowProc;
		wnd_class.cbClsExtra = 0;
		wnd_class.cbWndExtra = 0;
		wnd_class.hInstance = m_Core.hinst;
		wnd_class.hIcon = ::LoadIcon(NULL, IDI_APPLICATION);
		wnd_class.hCursor = ::LoadCursor(NULL, IDC_ARROW);
		wnd_class.hbrBackground = static_cast<HBRUSH>(::GetStockObject(BLACK_BRUSH));
  		wnd_class.lpszMenuName = NULL;
		wnd_class.lpszClassName = m_Core.name;

		if (!::RegisterClass(&wnd_class))
			return FALSE;
	}

	if (!m_Wnd) {

		m_Wnd = ::CreateWindow(m_Core.name, m_Core.name, WS_OVERLAPPEDWINDOW & ~WS_SIZEBOX,
			m_Pos.x, m_Pos.y, m_Size.cx, m_Size.cy,
			NULL, NULL, m_Core.hinst, NULL);

		if (!m_Wnd)
			return FALSE;
	}

#else

	// TODO:

#endif

	if (!Start())
		return FALSE;

	return TRUE;
}

VOID DApp::Exit(VOID)
{
	End();
}

VOID DApp::Run(VOID)
{
#ifdef _WIN32
	MSG msg;

	DVarClr(msg);
	::ShowWindow(m_Wnd, m_Core.show);
	::UpdateWindow(m_Wnd);

	BOOL loop = TRUE;

	while (TRUE) {

		if (::PeekMessage(&msg, NULL, 0, 0, PM_REMOVE)) {

			if (msg.message == WM_QUIT)
				break;

			::TranslateMessage(&msg);
			::DispatchMessage(&msg);

		} else {

			LARGE_INTEGER now;
			if (!::QueryPerformanceCounter(&now))
				continue;

			QWORD tick = (now.QuadPart - m_StartTime.QuadPart) / (m_Freq.QuadPart / 1000);
			if (loop && !Loop(tick)) {
				loop = FALSE;
				::PostQuitMessage(0);
			}

			::Sleep(10);
		}
	}

	m_RetCode = msg.wParam;
#else

#endif

	return;
}

INT DApp::Return(VOID)
{
	return m_RetCode;
}

BOOL DApp::Check(VOID) CONST
{
	if (!m_Core.name || !*m_Core.name)
		return FALSE;

#ifdef _WIN32
	if (!m_Core.hinst)
		return FALSE;
#else
	if (m_Core.argc <= 0)
		return FALSE;
#endif

	if (m_Pos.x < 0 || m_Pos.y < 0)
		return FALSE;

	if (m_Size.cx <= 0 || m_Size.cy <= 0)
		return FALSE;

	return TRUE;
}

/************************************************************************/
