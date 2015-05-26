#include <stdlib.h>
#include <stdio.h>
#include <locale.h>

#include <windows.h>
#include <mzfc_inc.h>
#include <acc_api.h>
#include <WlanEnableApi.h>
#include <CallNotifyApi.h>
//#include <ImagingHelper.h> 
#include <ShellNotifyMsg.h>
#include <ReadWriteIni.h>
//#include <UiToolbar_Text.h> 
#include <IFileBrowser.h>
#include <IFileBrowser_GUID.h>
#include <InitGuid.h>
#include <IMzUnknown.h>
#include <IMzUnknown_IID.h>
//#include <UiButton.h> 
#include <assert.h>
//#include <imaging.h>
#include <UsbNotifyApi.h>
#include <mzfc\creg.h>

#include <IMzSysFile.h>
#include <IPlayerCore.h>
#include <IPlayerCore_IID.h>
#include <PlayerCore_GUID.h>

#include <imixer.h>
#include <imixer_guid.h>


#include "sgf.h"
#include "utils.h"

// by liigo, 2009/5
// com.liigo_at_gmail.com
// http://blog.csdn.net/liigo

//-----------------------------------------------------------------------------
// copy from <imaging.h>
// ���include <imaging.h> �ᵼ�ºܶ����Ӵ���, ��������Ϊ <imgguids.h> ��û�б��ⱻ�ظ������������


#define M8_WEIQIPU  L"M8Χ����"
#define M8_WEIQIPU_VERINON  M8_WEIQIPU L" " L"v0.8.8"


#ifdef _WIN32_WCE
interface DECLSPEC_UUID("327ABDA9-072B-11D3-9D7B-0000F81EF32E")
#else
MIDL_INTERFACE("327ABDA9-072B-11D3-9D7B-0000F81EF32E")
#endif  // _WIN32_WCE
IImage : public IUnknown
{
public:

    // Get the device-independent physical dimension of the image
    //  in unit of 0.01mm

    STDMETHOD(GetPhysicalDimension)(
        OUT SIZE* size
        ) = 0;

    // Get basic image info

    STDMETHOD(GetImageInfo)(
        OUT /*ImageInfo*/void* imageInfo
        ) = 0;

    // Set image flags

    STDMETHOD(SetImageFlags)(
        IN UINT flags
        ) = 0;

    // Display the image in a GDI device context

    STDMETHOD(Draw)(
        IN HDC hdc,
        IN const RECT* dstRect,
        IN OPTIONAL const RECT* srcRect
        ) = 0;

    // Push image data into an IImageSink

    STDMETHOD(PushIntoSink)(
        IN /*IImageSink*/void* sink
        ) = 0;

    // Get a thumbnail representation for the image object

    STDMETHOD(GetThumbnail)(
        IN OPTIONAL UINT thumbWidth,
        IN OPTIONAL UINT thumbHeight,
        OUT IImage** thumbImage
        ) = 0;
};

//-----------------------------------------------------------------------------
//ȡEXE����Ŀ¼���ļ�������·��
//relativeFile: ���EXE����Ŀ¼���ļ���, ��: "abc.txt", "dir\abc.txt"
bool GetFullPathFileName(TCHAR* buffer, size_t bufferlen, const TCHAR* relativeFile)
{
	GetModuleFileName(MzGetInstanceHandle(), buffer, bufferlen);
	TCHAR* p = wcsrchr(buffer, L'\\');
	ASSERT(p);
	if(p)
	{
		*(p+1) = L'\0';
		wcscat(buffer, relativeFile);
		return true;
	}else{
		buffer[0] = L'\0';
	}
	return false;
}

HFONT CreateSymtemFont(int fontHeight)
{
	static HGDIOBJ s_hSysFont = 0;
	static LOGFONT s_font;
	if(s_hSysFont == 0)
	{
		s_hSysFont = GetStockObject(SYSTEM_FONT);
		GetObject(s_hSysFont, sizeof(s_font), &s_font);
		s_font.lfWidth = 0;
		s_font.lfQuality = CLEARTYPE_QUALITY;
	}

	LOGFONT font;
	memcpy(&font, &s_font, sizeof(font));
	font.lfHeight = fontHeight;
	return CreateFontIndirect(&font);
}

#include <windows.h>  
  
typedef BOOL (WINAPI *EnumerateFunc) (TCHAR* lpFileOrPath, void* pUserData);
  
void doFileEnumeration(TCHAR* lpPath, BOOL bRecursion, BOOL bEnumFiles, EnumerateFunc pFunc, void* pUserData)  
{  
  
    static BOOL s_bUserBreak = FALSE;  
    try{  
        //-------------------------------------------------------------------------  
        if(s_bUserBreak) return;  
          
        int len = wcslen(lpPath);  
        if(lpPath==NULL || len<=0) return;  
          
        //NotifySys(NRS_DO_EVENTS, 0,0);  
          
        TCHAR path[MAX_PATH];  
		wcscpy(path, lpPath);  
		if(lpPath[len-1] != _T('\\')) wcscat(path, _T("\\")); 
		wcscat(path, _T("*"));  
          
        WIN32_FIND_DATA fd;  
        HANDLE hFindFile = FindFirstFile(path, &fd);  
        if(hFindFile == INVALID_HANDLE_VALUE)  
        {  
            ::FindClose(hFindFile); return;  
        }  
          
        TCHAR tempPath[MAX_PATH]; BOOL bUserReture=TRUE; BOOL bIsDirectory;  
          
        BOOL bFinish = FALSE;  
        while(!bFinish)  
        {  
            wcscpy(tempPath, lpPath);  
            if(lpPath[len-1] != _T('\\')) wcscat(tempPath, _T("\\"));  
            wcscat(tempPath, fd.cFileName);  
              
            bIsDirectory = ((fd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) != 0);  
              
            //�����.��..  
            if( bIsDirectory  
				&& (wcscmp(fd.cFileName, _T("."))==0 || wcscmp(fd.cFileName, _T(".."))==0))   
            {         
                bFinish = (FindNextFile(hFindFile, &fd) == FALSE);  
                continue;  
            }  
              
            if(pFunc && bEnumFiles!=bIsDirectory)  
            {  
                bUserReture = pFunc(tempPath, pUserData);  
                if(bUserReture==FALSE)  
                {  
                    s_bUserBreak = TRUE; ::FindClose(hFindFile); return;  
                }  
            }  
              
            //NotifySys(NRS_DO_EVENTS, 0,0);  
              
            if(bIsDirectory && bRecursion) //����Ŀ¼  
            {  
                doFileEnumeration(tempPath, bRecursion, bEnumFiles, pFunc, pUserData);  
            }  
              
            bFinish = (FindNextFile(hFindFile, &fd) == FALSE);  
        }  
          
        ::FindClose(hFindFile);  
          
        //-------------------------------------------------------------------------  
    }catch(...){ ASSERT(0); return; }  
} 


//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------

//������ʾ���ݺܶ�Ľ�˵�ı�
class CExplainWnd: public CMzWndEx
{
	MZ_DECLARE_DYNAMIC(CExplainWnd);

private:
	UiToolbar_Text m_toolbar;

	enum MYIDS{ MYID_START = 100, MYID_TOOLBAR, };

public:
	CExplainWnd::CExplainWnd()
	{
	}

	virtual BOOL OnInitDialog()
	{
		if (!CMzWndEx::OnInitDialog())
		{
			return FALSE;
		}

		m_toolbar.SetID(MYID_TOOLBAR);
		m_toolbar.EnableLeftArrow(true);
		m_toolbar.SetButton(0, true, true, L"ȡ��");
		m_toolbar.SetButton(2, true, true, L"ȷ��");
		m_toolbar.SetPos(0, GetHeight()-MZM_HEIGHT_TEXT_TOOLBAR, GetWidth(), MZM_HEIGHT_TEXT_TOOLBAR);

		AddUiWin(&m_toolbar);

		return TRUE;
	}

	BOOL CreateWnd(HWND parentHwnd)
	{
		RECT rc = MzGetWorkArea();
		CMzWndEx::Create(0, 0, RECT_WIDTH(rc), RECT_HEIGHT(rc), parentHwnd);
		AnimateWindow(MZ_ANIMTYPE_ZOOM_IN, true);
		Show();
		return TRUE;
	}

	virtual void OnMzCommand(WPARAM wParam, LPARAM lParam)
	{
		WORD id = LOWORD(wParam);
		WORD notifyCode = HIWORD(wParam);

		switch(id)
		{
		case MYID_TOOLBAR:
			{
				int index = lParam;
				if(index == 0)
				{
					DestroyWindow();
					delete this;
				}
			}
		}
	}

};

MZ_IMPLEMENT_DYNAMIC(CExplainWnd)


//-----------------------------------------------------------------------------

#define APP_NAME L"Settings"

class SkinSetting
{
private:
	bool m_isValid;
	ImagingHelper m_backgroundImage, m_blackImage, m_whiteImage, m_blankImage;
	ImagingHelper m_blackNewImage, m_whiteNewImage;
	ImagingHelper m_blackNumsImage, m_whiteNumsImage;

	int m_boardLineCount;
	int m_boardLineWidth, m_boardLineInterval;
	int m_boardTopY, m_boardBottomY;
	int m_stoneWidth;
	int m_TopLeftStoneX, m_TopLeftStoneY;
	CMzString m_name, m_description, m_skinPath, m_skinId;
	CMzString m_luoziSoundFile;

	IPlayerCore_Play* m_pPlayCore;
	IMzSysFile* m_pSysFile;
	int m_soundVolume;

	void InitMembers()
	{
		m_isValid = false;

	}

	TCHAR* removeQuotes(TCHAR* str)
	{
		if(str == NULL || str[0] == L'\0') return str;
		int n = wcslen(str);
		if(str[0] == L'\"' && str[n-1] == L'\"')
		{
			str[n-1] = L'\0';
			return str + 1;
		}
		return str;
	}

	bool ReadSettingsFile(CMzString settingsFile, bool fullLoad)
	{
		//MzMessageBoxEx(0, settingsFile, NULL);
		InitMembers();
		
		TCHAR* appName = L"Settings";
		TCHAR* temp = NULL;
		IniReadString(appName, L"Name", &temp, settingsFile);
		m_name = removeQuotes(temp);
		free(temp);
		temp = NULL;
		IniReadString(appName, L"Description", &temp, settingsFile);
		m_description = removeQuotes(temp);
		free(temp);

		if(fullLoad)
		{
			IniReadInt(appName, L"BoardLineCount", (DWORD*) &m_boardLineCount, settingsFile);
			IniReadInt(appName, L"TopLeftStoneX", (DWORD*) &m_TopLeftStoneX, settingsFile);
			IniReadInt(appName, L"TopLeftStoneY", (DWORD*) &m_TopLeftStoneY, settingsFile);
			IniReadInt(appName, L"BoardLineWidth", (DWORD*) &m_boardLineWidth, settingsFile);
			IniReadInt(appName, L"BoardLineInterval", (DWORD*) &m_boardLineInterval, settingsFile);
			m_boardTopY = m_boardBottomY = 0; //��ĳЩƤ���п���δָ��������
			IniReadInt(appName, L"BoardTopY", (DWORD*) &m_boardTopY, settingsFile);
			IniReadInt(appName, L"BoardBottomY", (DWORD*) &m_boardBottomY, settingsFile);
			IniReadInt(appName, L"StoneWidth", (DWORD*) &m_stoneWidth, settingsFile);
		}
		/*
		TCHAR buff[1024];
		swprintf(buff, L"Size:%d, xy:%d,%d, w:%d, interval:%d, stonewith:%d", 
			m_boardLineCount, m_boardTopLeftX, m_boardTopLeftY, m_boardLineWidth, m_boardLineInterval, m_stoneWidth);
		MzMessageBoxEx(0, buff, L"", MB_OK);
		*/
		m_isValid = true;
		return true;
	}

public:
	SkinSetting()
	{
		InitMembers();

		m_pSysFile = NULL;
		m_pPlayCore = NULL;
		m_soundVolume = 50; //50%
	}

	void SetSysFileAndPlayerCore(IMzSysFile* pSysFile, IPlayerCore_Play* pPlayCore)
	{
		m_pSysFile = pSysFile;
		m_pPlayCore = pPlayCore;
	}

	//if fullLoad == false, only load the skin's name/description/path/id
	bool LoadSkin(const TCHAR* szSkinPath, bool fullLoad = true)
	{
		ASSERT(szSkinPath);
		m_skinPath = szSkinPath;

		if(szSkinPath[m_skinPath.Length() - 1] != L'\\')
		{
			m_skinPath = m_skinPath + L"\\";
		}

		if(fullLoad)
		{
			CMzString file = m_skinPath + L"background.png";
			m_backgroundImage.LoadImageW(file, true, true, true);
			file = m_skinPath + L"black.png";
			m_blackImage.LoadImageW(file, true, true, true);
			file = m_skinPath + L"white.png";
			m_whiteImage.LoadImageW(file, true, true, true);
			file = m_skinPath + L"blank.png";
			m_blankImage.LoadImageW(file, true, true, true);
			file = m_skinPath + L"black_new.png";
			m_blackNewImage.LoadImageW(file, true, true, true);
			file = m_skinPath + L"white_new.png";
			m_whiteNewImage.LoadImageW(file, true, true, true);
			file = m_skinPath + L"black_nums.png";
			m_blackNumsImage.LoadImageW(file, true, false, true);
			file = m_skinPath + L"white_nums.png";
			m_whiteNumsImage.LoadImageW(file, true, false, true);

			m_luoziSoundFile = m_skinPath + L"luozi.wav"; //todo: ���û�и��ļ�, ָ��Ĭ�����������ļ�
			if(m_pSysFile && m_pPlayCore)
			{
				m_pSysFile->SetName(m_luoziSoundFile);
				//m_pPlayCore->OpenFile(); //�˴����ûᵼ�������ڽ����̨�����
			}
		}

		//get the skin id
		CMzString temp = m_skinPath;
		TCHAR* pTempData = temp;
		pTempData[temp.Length()-1] = L'\0';
		pTempData = wcsrchr(pTempData, L'\\');
		if(pTempData)
			m_skinId = pTempData + 1;

		return ReadSettingsFile(m_skinPath + L"skin.ini", fullLoad);
	}

	const TCHAR* GetName() { return m_name; }
	const TCHAR* GetDiscription() { return m_description; }
	const TCHAR* GetPath() { return m_skinPath; }
	const TCHAR* GetId() { return m_skinId; }

	int GetStoneSize() { return m_stoneWidth; }

	//get the stone's center-point x,y
	bool GetStoneXY(int row, int col, int&x, int& y)
	{
		if(row < 1 || row > m_boardLineCount || col < 1 || col > m_boardLineCount)
		{
			ASSERT(0);
			x = y = -1000;
			return false;
		}

		x = m_TopLeftStoneX + (col - 1) * (m_boardLineInterval + m_boardLineWidth) + (m_boardLineWidth / 2);
		y = m_TopLeftStoneY + (row - 1) * (m_boardLineInterval + m_boardLineWidth) + (m_boardLineWidth / 2);
		return true;
	}

	//row,col: 1 - 19
	bool GetStoneImageXY(int row, int col, int&x, int& y)
	{
		if(GetStoneXY(row, col, x, y))
		{
			x -= m_stoneWidth / 2;
			y -= m_stoneWidth / 2;
			return true;
		}
		return false;
	}

	bool GetStoneRowColFromXY(int x, int y, int& row, int& col)
	{
		if(x < 0 || y < 0 || x >= 480 || y >=720)
		{
			row = col = -1; return false;
		}
		x -= m_TopLeftStoneX;
		y -= m_TopLeftStoneY;
		
		row = y / (m_boardLineInterval + m_boardLineWidth) + 1;
		if(y % (m_boardLineInterval + m_boardLineWidth) > m_boardLineInterval / 2)
			row++;

		col = x / (m_boardLineInterval + m_boardLineWidth) + 1;
		if(x % (m_boardLineInterval + m_boardLineWidth) > m_boardLineInterval / 2)
			col++;

		return (row >= 1 && row <= m_boardLineCount && col >= 1 && col <= m_boardLineCount);
	}

	void DrawImageXY(ImagingHelper& imageHelper, HDC hDC, int x, int y)
	{
		RECT rc;
		rc.left = x; rc.top = y;
		rc.right = x + m_stoneWidth /* imageHelper.GetImageWidth() */;
		rc.bottom = y + m_stoneWidth /* imageHelper.GetImageHeight() */;
		imageHelper.Draw(hDC, &rc);
	}

	TCHAR* GetLuoziSoundFile()
	{
		return m_luoziSoundFile;
	}

	void SetSoundVolume(int volume /* 0 - 100*/)
	{
		m_soundVolume = volume;
	}

	void PlayLuoziSound()
	{
		//PlaySound(m_luoziSoundFile, NULL, SND_FILENAME|SND_SYNC);

		if(m_pPlayCore)
		{
			m_pPlayCore->OpenFile();
			m_pPlayCore->SetVolume(m_soundVolume);
			m_pPlayCore->Play();
		}
	}

	bool DrawStoneImage(ImagingHelper& imageHelper, HDC hDC, int row, int col, bool playSound = false)
	{
		int x, y;
		if(GetStoneImageXY(row, col, x, y))
		{
			DrawImageXY(imageHelper, hDC, x, y);
			if(playSound) PlayLuoziSound();
			return true;
		}
		return false;
	}

	bool DrawBlackStone(HDC hDC, int row, int col, bool playSound = false) 
	{
		return DrawStoneImage(m_blackImage, hDC, row, col, playSound);
	}

	bool DrawWhiteStone(HDC hDC, int row, int col, bool playSound = false) 
	{
		return DrawStoneImage(m_whiteImage, hDC, row, col, playSound);
	}

	bool DrawBlackNewStone(HDC hDC, int row, int col, bool playSound = false) 
	{
		return DrawStoneImage(m_blackNewImage, hDC, row, col, playSound);
	}

	bool DrawWhiteNewStone(HDC hDC, int row, int col, bool playSound = false) 
	{
		return DrawStoneImage(m_whiteNewImage, hDC, row, col, playSound);
	}

	bool DrawBlackNum(HDC hDC, int row, int col, int num)
	{
		return DrawStoneNumImage(m_blackNumsImage, hDC, num, row, col);
	}

	bool DrawWhiteNum(HDC hDC, int row, int col, int num)
	{
		return DrawStoneNumImage(m_whiteNumsImage, hDC, num, row, col);
	}

	bool DrawStoneNumImage(ImagingHelper& imageHelper, HDC hDC, int num, int row, int col)
	{
		int w = imageHelper.GetImageWidth() / 10;
		if(num < 1 || num > 10 * (imageHelper.GetImageHeight() / w))
			return false;
		int x, y;
		if(GetStoneXY(row, col, x, y))
		{
			int dx = w * ((num-1) % 10);
			int dy = w * ((num-1) / 10);
			//��M8����,1mm��Լ10������,��ÿ����0.1mm. ����srcRect�������ת��Ϊ0.01mm��λ,��10����. 
			//��GetDeviceCaps()�ƺ������������򵥶���ȷ�ء��Ժ���ֲ�������ֻ�ʱ��ע�������
			const int n = 35;
			RECT srcRect = { dx*n, dy*n, (dx+w)*n, (dy+w)*n };
			RECT dstRect = { x - w/2, y - w/2, x + w/2, y + w/2 };
			IImage* pImage = imageHelper.GetIIMage();
			pImage->Draw(hDC, &dstRect, &srcRect);
			return true;
		}
		return false;
	}

	bool DrawBlankStone(HDC hDC, int row, int col) 
	{
		return DrawStoneImage(m_blankImage, hDC, row, col, false);
	}

	void DrawBackground(HDC hDC)
	{
		static RECT rc = { 0, 0, 480, 720 };
		m_backgroundImage.Draw(hDC, &rc);
	}

	int GetBoardTopY()
	{
		return m_boardTopY > 0 ? m_boardTopY : (m_TopLeftStoneY - m_boardLineInterval / 2);
	}

	int GetBoardBottomY()
	{
		if(m_boardBottomY > 0)
			return m_boardBottomY;
		else
			return (m_TopLeftStoneY + m_boardLineCount * (m_boardLineInterval + m_boardLineWidth) - m_boardLineInterval / 2);
	}
};

//-----------------------------------------------------------------------------

class UiList_Skin : public UiList
{
private:
	HWND m_OnSelectListenerHwnd;
	int m_OnSelectListenerMsg;
	HFONT m_hFont1, m_hFont2;
public:
	UiList_Skin()
	{
		m_OnSelectListenerHwnd = 0;
		m_OnSelectListenerMsg = 0;

		m_hFont1 = CreateSymtemFont(-30);
		m_hFont2 = CreateSymtemFont(-22);
	}

	~UiList_Skin()
	{
		DeleteObject(m_hFont1);
		DeleteObject(m_hFont2);
	}

	//Ϊ����߲��������ԣ�������������Ż�����ǰ���������壻˫�����ͼ��ǰ��Ч��������һЩ��������ǱȽϿ���liigo, 20091018
	void DrawItem(HDC hdcDst, int nIndex, RECT* prcItem, RECT *prcWin, RECT *prcUpdate)
	{
		RECT rc = *prcItem;
		int rcWidth = RECT_WIDTH(rc);
		int rcHeight = RECT_HEIGHT(rc);
		HDC hBufferDC = CreateCompatibleDC(hdcDst);
		HBITMAP hBufferBmp = CreateCompatibleBitmap(hdcDst, rcWidth, rcHeight);
		HBITMAP hOldBmp = (HBITMAP) SelectObject(hBufferDC, hBufferBmp);
		HDC hDC = hBufferDC;

		// ����ѡ����ĸ�������
		if (nIndex == GetSelectedIndex())
		{
			RECT tempRC = { 0, 0, rcWidth, rcHeight };
			MzDrawSelectedBg(hDC, &tempRC);
		}
		else
			BitBlt(hDC, 0,0, rcWidth,rcHeight, hdcDst, rc.left,rc.top, SRCCOPY);

		// �����ı�
		CMzString s;
		RECT rcText = *prcItem;
		ListItem* pItem = GetItem(nIndex);
		if (pItem && pItem->Data)
		{
			SkinSetting* pSkin = (SkinSetting*) pItem->Data;

			rcText.left = 5;
			rcText.top = 10;

			SetBkMode(hDC, TRANSPARENT);

			s = s + L"Ƥ����" + pSkin->GetName();
			int fontsize = 30;
			HGDIOBJ hOldFont = SelectObject(hDC, m_hFont1);
			rcText.bottom = rcText.top + fontsize;
			MzDrawText(hDC, s, &rcText, DT_LEFT|DT_VCENTER|DT_SINGLELINE|DT_END_ELLIPSIS);

			rcText.top += fontsize + 5;
			fontsize = 20;
			hOldFont = SelectObject(hDC, m_hFont2);
			s = L"˵����"; s = s + pSkin->GetDiscription();
			rcText.bottom = rcText.top + fontsize;
			MzDrawText(hDC, s, &rcText, DT_LEFT|DT_VCENTER|DT_SINGLELINE|DT_END_ELLIPSIS);

			rcText.top += fontsize + 2; rcText.bottom = rcText.top + fontsize;
			s = L"·����"; s = s + pSkin->GetPath();
			MzDrawText(hDC, s, &rcText, DT_LEFT|DT_VCENTER|DT_SINGLELINE|DT_END_ELLIPSIS);
			
			SelectObject(hDC, hOldFont);
		}

		BitBlt(hdcDst, rc.left,rc.top, rcWidth,rcHeight, hBufferDC, 0,0, SRCCOPY);

		SelectObject(hBufferDC, hOldBmp);
		DeleteObject(hBufferBmp);
		DeleteDC(hBufferDC);
	}

	void SetOnSelectListener(HWND hwnd, int message)
	{
		m_OnSelectListenerHwnd = hwnd;
		m_OnSelectListenerMsg = message;
	}

	virtual int OnLButtonDown(UINT fwKeys, int xPos, int yPos)
	{
		// ��ÿؼ������ſ�ǰ��һЩ�������״̬���ڵ��� UiLst::OnLButtonUp()��
		//bool b2 = IsMouseMoved();
		int Ret = UiList::OnLButtonDown(fwKeys, xPos, yPos);
		//if(!b2)
		{
			int index = CalcIndexOfPos(xPos, yPos);
			if(index >= 0)
			{
				int old_sel = GetSelectedIndex();
				SetSelectedIndex(index);
				InvalidateItem(index);
				if(old_sel >= 0) InvalidateItem(old_sel);
			}
		}
		return Ret;
	}

	virtual int OnLButtonUp(UINT fwKeys, int xPos, int yPos)
	{
		// ��ÿؼ������ſ�ǰ��һЩ�������״̬���ڵ��� UiLst::OnLButtonUp()��
		bool b1 = IsMouseDownAtScrolling();
		bool b2 = IsMouseMoved();

		int Ret = UiList::OnLButtonUp(fwKeys, xPos, yPos);

		if((!b1) && (!b2))
		{
			if(m_OnSelectListenerHwnd)
			{
				int index = CalcIndexOfPos(xPos, yPos);
				if(index >= 0)
				{
					::PostMessage(m_OnSelectListenerHwnd, m_OnSelectListenerMsg, index, 0);
				}
			}
		}
		return Ret;
	}
};

BOOL fnOnEnumerateSkinIni(TCHAR* lpFileOrPath, void* pUserData);

//liigo 20091018: Ϊ���л�Ƥ����û�ٷ���ţ����
class CBrowseSkins: public CMzWndEx
{
	MZ_DECLARE_DYNAMIC(CBrowseSkins);

private:
	UiList_Skin m_skinList;
	UiToolbar_Text m_toolbar;

	const CMzString& m_skinsPath;
	SkinSetting* m_pSelectedSkin;
	
	enum { MYID_SKIN_LIST = 100, MYID_TOOLBAR, };
	static const int ON_SELECT_SKIN_MSG = 0x123456;

public:
	CBrowseSkins(const CMzString& skinsPath)
		: m_skinsPath(skinsPath)
	{
		m_pSelectedSkin = NULL;
	}

	virtual BOOL OnInitDialog()
	{
		if (!CMzWndEx::OnInitDialog())
		{
			return FALSE;
		}

		m_skinList.SetPos(0, 0, GetWidth(), GetHeight());
		m_skinList.SetID(MYID_SKIN_LIST);
		m_skinList.SetItemHeight(90);
		m_skinList.SetOnSelectListener(m_hWnd, ON_SELECT_SKIN_MSG);
		AddUiWin(&m_skinList);

		m_toolbar.SetID(MYID_TOOLBAR);
		m_toolbar.EnableLeftArrow(true);
		m_toolbar.SetButton(0, true, true, L"ȡ��");
		//m_toolbar.SetButton(2, true, true, L"ȷ��");
		m_toolbar.SetPos(0, GetHeight()-MZM_HEIGHT_TEXT_TOOLBAR, GetWidth(), MZM_HEIGHT_TEXT_TOOLBAR);
		AddUiWin(&m_toolbar);

		Show();
		doFileEnumeration(m_skinsPath, TRUE, TRUE, fnOnEnumerateSkinIni, this);

		return TRUE;
	}

	void OnEnumerateSkinIni(TCHAR* lpFileOrPath)
	{
		TCHAR* pLastSlash = wcsrchr(lpFileOrPath, L'\\');
		if(pLastSlash)
		{
			*pLastSlash = L'\0';
			SkinSetting* pSkin = new SkinSetting();
			if(pSkin->LoadSkin(lpFileOrPath, false)) //not full load
			{
				ListItem li;
				li.Data = pSkin;
				m_skinList.AddItem(li);
			}
			else
				delete pSkin;
		}
	}

	LRESULT MzDefWndProc(UINT message, WPARAM wParam, LPARAM lParam)
	{
		int nID = LOWORD(wParam);
        int nNotify = HIWORD(wParam);
        int x = LOWORD(lParam);
        int y = HIWORD(lParam);

        if(message == ON_SELECT_SKIN_MSG)
		{
			int index = wParam;
			deleteSkins(index);
			m_pSelectedSkin = (SkinSetting*) (m_skinList.GetItem(index)->Data);
			EndModal(ID_OK);
			return 0;
		}
		return CMzWndEx::MzDefWndProc(message,wParam,lParam);
	}

	SkinSetting* GetSelectedSkin() { return m_pSelectedSkin; }

	virtual void OnMzCommand(WPARAM wParam, LPARAM lParam)
	{
		WORD id = LOWORD(wParam);
		WORD notifyCode = HIWORD(wParam);

		switch(id)
		{
		case MYID_TOOLBAR:
			{
				int index = lParam;
				deleteSkins(-1);
				EndModal(ID_CANCEL);
				__assert(m_pSelectedSkin == NULL);
				break;
			}
		}
	}

private:
	void deleteSkins(int excludeIndex)
	{
		SkinSetting* pSkin = NULL;
		for(int i = m_skinList.GetItemCount() - 1; i >= 0; i--)
		{
			if(i != excludeIndex)
			{
				pSkin = (SkinSetting*) (m_skinList.GetItem(i)->Data);
				delete pSkin;
			}
		}
	}
};

MZ_IMPLEMENT_DYNAMIC(CBrowseSkins)

BOOL fnOnEnumerateSkinIni(TCHAR* lpFileOrPath, void* pUserData)
{
	TCHAR* pLastSlash = wcsrchr(lpFileOrPath, L'\\');
	if(pLastSlash && (wcsicmp(pLastSlash, L"\\skin.ini") == 0))
	{
		CBrowseSkins* pBrowseSkins = (CBrowseSkins*) pUserData;
		pBrowseSkins->OnEnumerateSkinIni(lpFileOrPath);
	}
	return TRUE;
}

//-----------------------------------------------------------------------------

#define APP_NAME L"Settings"

class CSettingWnd: public CMzWndEx
{
	MZ_DECLARE_DYNAMIC(CSettingWnd);

private:
	const TCHAR* m_iniFileName;
	UiScrollWin m_scrollWin;
	//UiButton m_SoundOn;
	//UiButton m_NextStoneOnRight;
	//UiButton m_SelectSkin;

	UiCaption m_Caption1;
	UiCaption m_Caption2;
	UiButtonEx m_SoundOnButton;
	UiButtonEx m_NextStoneOnRightButton;
	UiButtonEx m_SelectSkin;

	UiToolbar_Text m_toolbar;
	bool m_SoundOn, *m_pVarSoundOn;
	bool m_NextStoneOnRight, *m_pVarNextStoneOnRight;
	UiToolbar_Text* m_pMainToolBar;
	CMzString& m_varSkinPath;
	CMzString m_skinsPath, m_skinName, m_newSkinPath, m_newSkinId;
	SkinSetting& m_varSkin;

	enum MYIDS{ MYID_START = 100, MYID_SCROLL_WIN, MYID_TOOLBAR, MYID_SOUND_ON, MYID_NEXT_STONE_ON_RIGHT, MYID_SELECT_SKIN, };

public:
	CSettingWnd::CSettingWnd(const TCHAR* iniSettingFileName, bool* pVarSoundOn, bool* pVarNextStoneOnRight, 
							 const CMzString& skinsPath, CMzString& skinPath,CMzString skinName, SkinSetting& varSkin, UiToolbar_Text* pMainToolBar = NULL)
		: m_skinsPath(skinsPath), m_varSkinPath(skinPath), m_varSkin(varSkin)
	{
		m_iniFileName = iniSettingFileName;
		m_pVarSoundOn = pVarSoundOn;
		m_SoundOn = *pVarSoundOn;
		m_pVarNextStoneOnRight = pVarNextStoneOnRight;
		m_NextStoneOnRight = *pVarNextStoneOnRight;
		m_pMainToolBar = pMainToolBar;
		m_skinName = skinName;
	}

	virtual BOOL OnInitDialog()
	{
		if (!CMzWndEx::OnInitDialog())
		{
			return FALSE;
		}

		m_scrollWin.SetID(MYID_SCROLL_WIN);
		m_scrollWin.SetPos(0, 0, GetWidth(), GetHeight());
		AddUiWin(&m_scrollWin);

		int y = 0;
		m_Caption1.SetPos(0, y, GetWidth(), MZM_HEIGHT_CAPTION);
		m_Caption1.SetText(L"��������");
		m_scrollWin.AddChild(&m_Caption1);
		y += MZM_HEIGHT_CAPTION;

		m_SoundOnButton.SetPos(0, y, GetWidth(), MZM_HEIGHT_BUTTONEX);
		m_SoundOnButton.SetID(MYID_SOUND_ON);
		m_SoundOnButton.SetButtonType(MZC_BUTTON_LINE_BOTTOM);
		SetUiButtonExRightArrowImage(m_SoundOnButton);
		m_SoundOnButton.SetText(L"����������");
		m_SoundOnButton.SetText2(m_SoundOn ? L"��" : L"�ر�");
		m_scrollWin.AddChild(&m_SoundOnButton);
		y += MZM_HEIGHT_BUTTONEX;

		m_NextStoneOnRightButton.SetPos(0, y, GetWidth(), MZM_HEIGHT_BUTTONEX);
		m_NextStoneOnRightButton.SetID(MYID_NEXT_STONE_ON_RIGHT);
		m_NextStoneOnRightButton.SetButtonType(MZC_BUTTON_LINE_BOTTOM);
		SetUiButtonExRightArrowImage(m_NextStoneOnRightButton);
		m_NextStoneOnRightButton.SetText(L"\"��һ��\"��");
		m_NextStoneOnRightButton.SetText2(m_NextStoneOnRight ? L"���ұ�" : L"�����");
		m_scrollWin.AddChild(&m_NextStoneOnRightButton);
		y += MZM_HEIGHT_BUTTONEX;

		m_Caption2.SetPos(0, y, GetWidth(), MZM_HEIGHT_CAPTION);
		m_Caption2.SetText(L"Ƥ������");
		m_scrollWin.AddChild(&m_Caption2);
		y += MZM_HEIGHT_CAPTION;

		m_SelectSkin.SetPos(0, y, GetWidth(), MZM_HEIGHT_BUTTONEX);
		m_SelectSkin.SetID(MYID_SELECT_SKIN);
		m_SelectSkin.SetButtonType(MZC_BUTTON_LINE_BOTTOM);
		SetUiButtonExRightArrowImage(m_SelectSkin);
		m_SelectSkin.SetText(L"��ǰƤ����");
		m_SelectSkin.SetText2(m_skinName);
		m_scrollWin.AddChild(&m_SelectSkin);
		y += MZM_HEIGHT_BUTTONEX;

		m_toolbar.SetID(MYID_TOOLBAR);
		m_toolbar.EnableLeftArrow(true);
		m_toolbar.SetButton(0, true, true, L"ȡ��");
		m_toolbar.SetButton(2, true, true, L"ȷ��");
		m_toolbar.SetPos(0, GetHeight()-MZM_HEIGHT_TEXT_TOOLBAR, GetWidth(), MZM_HEIGHT_TEXT_TOOLBAR);

		AddUiWin(&m_toolbar);

		return TRUE;
	}

	BOOL CreateWnd(HWND parentHwnd)
	{
		RECT rc = MzGetWorkArea();
		CMzWndEx::Create(0, 0, RECT_WIDTH(rc), RECT_HEIGHT(rc), parentHwnd);
		AnimateWindow(MZ_ANIMTYPE_SCROLL_BOTTOM_TO_TOP_2, true);
		Show();
		return TRUE;
	}

	virtual void OnMzCommand(WPARAM wParam, LPARAM lParam)
	{
		WORD id = LOWORD(wParam);
		WORD notifyCode = HIWORD(wParam);

		switch(id)
		{
		case MYID_SOUND_ON:
			m_SoundOn = !m_SoundOn;
			m_SoundOnButton.SetText2(m_SoundOn ? L"��" : L"�ر�");
			m_SoundOnButton.Invalidate();
			break;
		case MYID_NEXT_STONE_ON_RIGHT:
			m_NextStoneOnRight = !m_NextStoneOnRight;
			m_NextStoneOnRightButton.SetText2(m_NextStoneOnRight ? L"���ұ�" : L"�����");
			m_NextStoneOnRightButton.Invalidate();
			break;
		case MYID_SELECT_SKIN:
			{
				//CBrowseSkins* pBrowseSkins = new CBrowseSkins(m_newskinPath, m_SelectSkin);
				//pBrowseSkins->CreateWnd(m_hWnd);
				//pBrowseSkins->Show();
				CBrowseSkins browseSkins(m_skinsPath);
				RECT rc = MzGetWorkArea();
				browseSkins.CreateModalDialog(rc.left,rc.top, RECT_WIDTH(rc), RECT_HEIGHT(rc), m_hWnd);
				browseSkins.SetAnimateType_Show(MZ_ANIMTYPE_SCROLL_RIGHT_TO_LEFT_PUSH);
				browseSkins.SetAnimateType_Hide(MZ_ANIMTYPE_SCROLL_LEFT_TO_RIGHT_PUSH);
				if(browseSkins.DoModal() == ID_OK)
				{
					SkinSetting* pSkin = browseSkins.GetSelectedSkin();
					m_SelectSkin.SetText2(pSkin->GetName());
					m_SelectSkin.Invalidate();
					m_newSkinPath = pSkin->GetPath();
					m_newSkinId = pSkin->GetId();
					delete pSkin;
				}
				break;
			}
		case MYID_TOOLBAR:
			{
				int index = lParam;
				if(index == 2)
				{
					bool b1 = IniWriteInt(APP_NAME, L"SoundOn", m_SoundOn ? 1 : 0, (TCHAR*)m_iniFileName);
					bool b2 = IniWriteInt(APP_NAME, L"NextStoneOnRight", m_NextStoneOnRight ? 1 : 0, (TCHAR*)m_iniFileName);
					bool b3 = true, loadNewSkin = false;
					if(m_newSkinId.Length() > 0)
					{
						b3 = IniWriteString(APP_NAME, L"Skin", (TCHAR*)m_newSkinId, (TCHAR*)m_iniFileName);
						loadNewSkin = true;
					}
					if(b1 && b2 && b3)
					{
						*m_pVarSoundOn = m_SoundOn;
						*m_pVarNextStoneOnRight = m_NextStoneOnRight;
						if(loadNewSkin)
						{
							m_varSkinPath = m_newSkinPath;
							m_varSkin.LoadSkin(m_newSkinPath);
						}
						if(m_pMainToolBar)
						{
							m_pMainToolBar->SetButton(0, true, true, *m_pVarNextStoneOnRight ? L"��һ��" : L"��һ��");
							m_pMainToolBar->SetButton(2, true, true, *m_pVarNextStoneOnRight ? L"��һ��" : L"��һ��");
						}
					}
					else
					{
						static int s_FailedCount = 0;
						s_FailedCount++;
						MzMessageBoxEx(m_hWnd, L"����ʧ�ܣ�", NULL);
						if(s_FailedCount <= 2)
							return;
						else
							s_FailedCount = 0;
					}
				}
				AnimateWindow(MZ_ANIMTYPE_SCROLL_TOP_TO_BOTTOM_1, false);
				DestroyWindow();
				delete this;
				break;
			}
		}
	}

private:
	void SetUiButtonExRightArrowImage(UiButtonEx& buttonex)
	{
		ImagingHelper *imgArrow = ImagingHelper::GetImageObject(GetMzResModuleHandle(),MZRES_IDR_PNG_ARROW_RIGHT, true);
		buttonex.SetImage2(imgArrow);
		buttonex.SetImageWidth2(imgArrow->GetImageWidth());
		buttonex.SetShowImage2(true);
	}

};

MZ_IMPLEMENT_DYNAMIC(CSettingWnd)

//-----------------------------------------------------------------------------


enum MY_IDS 
{
	MY_IDC_EXIT = 101,
	MY_IDC_TOOLBAR,
};

enum StoneColor{ SC_BLACK = -1, SC_WHITE = 1, SC_BLANK = 0 };

struct LabelInfo
{
	char row, col;
	const TCHAR* label;
};

struct StoneExtraInfo
{
	const TCHAR* szExplain;
	BufferedMem labelInfos; //array of LabelInfo

	StoneExtraInfo() { szExplain = NULL; }
};

struct StoneInfo
{
	char row, col;
	StoneColor color;
	int stoneIndex;
	const TCHAR* szExplain;
	StoneExtraInfo* extraInfo;

	StoneInfo() { row = col = 0; color = SC_BLANK; szExplain = NULL; extraInfo = NULL; }
};

struct StoneIndexInfo
{
	int posIndex; // contains row and col information
	int stoneIndex;
};

struct GameInfo
{
	CMzString ev; //�������� (Event)
	CMzString dt; //���� (Date)
	CMzString re, result; //��� (Result)
	CMzString pb; //�ڷ� (Player of Black)
	CMzString pw; //�׷� (Player of White)
	CMzString br; //�ڷ�ͷ�� (Black Rank)
	CMzString wr; //�׷�ͷ�� (White Rank)
	//CMzString us; //���췽?
	//CMzString so; //������Դ?
	CMzString km; //��Ŀ (Komi)
	int sz; //���̴�С (Size)
	int gm; //������� (Game) (Ĭ��1ΪΧ��)

	GameInfo() { Init(); } 

	void Init()
	{
		ev = dt = re = pb = pw = br = wr = km = L"";
		sz = 19; gm = 1;
		result = L"ʤ��δ֪";
	}
};

static inline bool StrEquals(const char* s1, const char* s2)
{
	return (_stricmp(s1, s2) == 0); //�����ִ�Сд
}

/*
//�˺�����Ӣ��ϵͳ�²��ܴ��������ַ�����M8ϵͳ�µ�CRT�ƺ�û��setlocale()����
static CMzString charsToString(const char* s)
{
	CMzString r;
	int n = mbstowcs(NULL, s, 0);
	if(n > 0)
	{
		TCHAR* p = (TCHAR*) malloc( (n+1) * sizeof(WCHAR) );
		p[n] = L'\0';
		int nn = mbstowcs(p, s, n);
		__assert(nn == n);
		r = p;
		free(p);
	}
	return r;
}
*/

//ȷ����Ӣ��ϵͳ��Ҳ��ת��SGF�ļ��е������ַ���Unicode
static int cncharsToWChar(const char* cnchars, WCHAR* wchars_buffer, int wchar_buffer_size)
{
	if(wchars_buffer == NULL)
		return MultiByteToWideChar(936 /*gb2312*/, 0, cnchars, -1, NULL, 0);
	else
		return MultiByteToWideChar(936 /*gb2312*/, 0, cnchars, -1, wchars_buffer, wchar_buffer_size);
}

static CMzString charsToString(const char* s)
{
	CMzString r;
	int n = cncharsToWChar(s, NULL, 0);
	if(n > 0)
	{
		WCHAR* p = (WCHAR*) malloc((n+1) * sizeof(WCHAR));
		cncharsToWChar(s, p, n+1);
		p[n] = L'\0';
		r = p;
		free(p);
	}
	return r;
}

//>= 1
int PointCharToRowOrCol(char c)
{
	__assert(c >= 'a' && c <= 's');
	return c - 'a' + 1;
}

//���ʧ�ܷ���false,������color=SC_BLANK, col=row=0
static bool SetStoneRowCol(StoneInfo& stone, const char* szMove)
{
	__assert(szMove && ( szMove[0] == '\0' || strlen(szMove) == 2)); //szMoveΪ�ձ�ʾ��Ȩ
	if(szMove[0] && szMove[1])
	{
		stone.col = PointCharToRowOrCol(szMove[0]);
		stone.row = PointCharToRowOrCol(szMove[1]);
		return true;
	}
	else
	{
		stone.color = SC_BLANK;
		stone.col = stone.row = 0;
		return false; //szMoveΪ�ձ�ʾ��Ȩ���������Ϊ�Ƿ�
	}
}

int RowColToIndex(int row, int col)
{
	assert(row >= 1 && row <= 19);
	assert(col >= 1 && col <= 19);
	return (row - 1) * 19 + col - 1;
}

void IndexToRowCol(int index, int& row, int& col)
{
	assert(index >= 0 && index <= 360);
	row = index / 19 + 1;
	col = index % 19 + 1;
}

static void SGF_OnProperty(SGFParseContext* pContext, const char* szID, const char* szValue);

static void SGF_OnTree(SGFParseContext* pContext, const char* szTreeHeader, int treeIndex)
{
	TCHAR buf[32];
	wsprintf(buf, L"onTree %d", treeIndex);
	MzMessageBoxEx(0, buf, NULL, MB_OK);
}

static int GetSysRingVolume()
{
	CMzRegKey regKey;
	regKey.Open(HKEY_CURRENT_USER, TEXT("ControlPanel\\Volume"));
	DWORD volume = 0;
	LPCTSTR valueName = TEXT("UsrVolume");
	if (ERROR_SUCCESS != regKey.QueryValue(volume, valueName))
	{
		//regKey.SetValue(DEFAULT_VOLUME, valueName);
		return 50;
	}
	else
	{
		return volume;
	}
}

static int myatoi(TCHAR* sz)
{
	int len = wcslen(sz);
	int ret = 0;
	for(int i = 0; i < len; i++)
	{
		CHAR c = sz[i];
		if(c < L'0' || c > L'9') return 0;
		ret = ret * 10 + (c - L'0');
	}
	return ret;
}

class CWeiqiPuMainWnd: public CMzWndEx
{
	MZ_DECLARE_DYNAMIC(CWeiqiPuMainWnd);
private:
	//UiMultiLineEditPro m_text;
	UiToolbar_Text m_toolbar;

	static const int m_fileNameBufferLen = 1024;
	TCHAR m_fileNameBuffer[m_fileNameBufferLen + 1];
	CMzString m_skinsPath, m_skinPath;
	CMzString m_mainSettingsFile;
	CMzString m_sgfPath, m_sgfFile;

	SkinSetting m_skin;

	int m_lbuttonDownX, m_lbuttonDownY;

	BufferedMem m_stonesMem;
	StoneInfo* m_pStones;
	int m_stoneCount;
	int m_stoneIndex; // 0, 1,2,3, ... -1 is the init value
	int m_board[19][19]; //the 0-based index of m_pStones, -1 means no stone here

	BufferedMem m_killedStonesMem;
	BufferedMem** m_killedStones; //BufferedMem*����,��СΪm_stoneCount

	MemBlocks m_stringContainer; // a number of string's content
	TCHAR* m_current_explain;
	TCHAR* m_game_explain; //before the first stone
	int m_numOfExplain;

	bool m_isInDapuState;
	bool m_SoundOn, m_NextStoneOnRight;
	bool m_blackFirst;

	GameInfo m_gameInfo;
	CMzString m_gameInfoString;

	int m_usbNotifyMsg;
	int m_allkeyEventMsg;
	int m_soundVolume;
	HANDLE m_eventHandle;

	IPlayerCore_Play* m_pPlayCore;
	IMzSysFile* m_pSysFile;


public:
	virtual BOOL OnInitDialog()
	{
		//ӦMStoreҪ�󣬱��뵥ʵ������
		m_eventHandle = CreateEvent(NULL, FALSE, FALSE, L"Global\\M8WeiqiPu_by_liigo");
		if(m_eventHandle && GetLastError() == ERROR_ALREADY_EXISTS)
		{
			HWND hWnd = FindWindowW(NULL, M8_WEIQIPU);
			if(hWnd)
			{
				SetForegroundWindow(hWnd);
				SetActiveWindow(hWnd);
				::ShowWindow(hWnd, SW_SHOWNORMAL);
				::UpdateWindow(hWnd);
			}
			PostQuitMessage(0);
			return FALSE;
		}
		else
			SetWindowText(M8_WEIQIPU);

		// Must all the Init of parent class first!
		if (!CMzWndEx::OnInitDialog())
		{
			return FALSE;
		}

		m_stoneCount = 0;
		m_stoneIndex = -1;
		m_pStones = (StoneInfo*)m_stonesMem.GetData();
		m_killedStones = NULL;
		m_isInDapuState = false;
		m_current_explain = m_game_explain = NULL;
		m_numOfExplain = 0;
		m_blackFirst = true;
		CleanBoard(true);

		//��ʼ�������������
		m_pSysFile = NULL;
		m_pPlayCore = NULL;
		if(SUCCEEDED( CoCreateInstance(CLSID_PlayerCore, NULL, CLSCTX_INPROC_SERVER, IID_MZ_SysFile, (void **)&m_pSysFile)))
		{
			m_pSysFile->SetParentWnd(m_hWnd);
			if( !SUCCEEDED(m_pSysFile->QueryInterface( IID_PlayerCore_Play, (void**)&m_pPlayCore)) )
			{
				m_pSysFile->Release();
				m_pSysFile = NULL;
			}
			m_pPlayCore->SetNotify(m_hWnd, WM_USER + 15);
		}
		m_skin.SetSysFileAndPlayerCore(m_pSysFile, m_pPlayCore); //m_skin.PlayLuoziSound()�����ڴ˵���


		//��ȡƤ������
		GetFullPathFileName(m_fileNameBuffer, m_fileNameBufferLen, L"M8WeiqiPu.ini");
		m_mainSettingsFile = m_fileNameBuffer;
		TCHAR* skinId = NULL;
		IniReadString(L"Settings", L"Skin", &skinId, m_mainSettingsFile);
		GetFullPathFileName(m_fileNameBuffer, m_fileNameBufferLen, L"skins\\");
		m_skinsPath = m_fileNameBuffer;
		m_skinPath = m_skinsPath + skinId + L"\\";
		m_skin.LoadSkin(m_skinPath); //����Ƥ���ļ�
		free(skinId);

		//��ȡ��������
		/*
		IMixer* mixer = NULL;
		if(SUCCEEDED( CoCreateInstance(CLSID_Mixer, NULL, CLSCTX_INPROC_SERVER, IID_MZ_Mixer, (void **)&mixer)))
		{
			TCHAR info[128];
			MzMessageBox(m_hWnd, L"create mixer ok", NULL, 0);
			BOOL bOK = mixer->OpenMixerDevice(m_hWnd);
			MzMessageBox(m_hWnd, bOK ? L"open mixer ok" : L"open error", NULL, 0);
			//mixer->SetRecordSourceSelected(MIC_PHONE_1);
			swprintf(info, L"speeker: %d, mic1: %d", mixer->GetSpeakerVolume(), 0 && mixer->GetMic1Volume()); //GetSpeakerVolume() �ܷ��ع̶�ֵ?
			MzMessageBox(m_hWnd, info, NULL, 0);
			mixer->Release();
		}
		*/
		/*
		TCHAR* volumeStr = NULL;
		IniReadString(L"Settings", L"Volume", &volumeStr, m_mainSettingsFile);
		if(volumeStr == NULL || volumeStr[0] == L'\0' || volumeStr[0] < L'0' || volumeStr[0] > L'9')
		{
			m_soundVolume = GetSysRingVolume();
		}
		else
		{
			m_soundVolume = myatoi(volumeStr);
		}
		free(volumeStr);
		*/
		m_soundVolume = GetSysRingVolume();
		//TCHAR buff[32]; wsprintf(buff, L"volum: %d", m_soundVolume);
		//MzMessageBox(m_hWnd, buff, NULL, 0);
		if(m_soundVolume < 0) m_soundVolume = 0;
		if(m_soundVolume > 100) m_soundVolume = 100;
		m_skin.SetSoundVolume(m_soundVolume);

		//��ȡ����ѡ���������/��һ�Ӱ�ťλ�ã�
		DWORD inivalue = 1;
		IniReadInt(APP_NAME, L"SoundOn", &inivalue, m_mainSettingsFile);
		m_SoundOn = (inivalue != 0);
		IniReadInt(APP_NAME, L"NextStoneOnRight", &inivalue, m_mainSettingsFile);
		m_NextStoneOnRight = (inivalue != 0);

		//���ù�����
		m_toolbar.SetID(MY_IDC_TOOLBAR);
		m_toolbar.SetButton(0, true, true, L"�˵�");
		m_toolbar.SetButton(2, true, true, L"������");
		m_toolbar.SetPos(0, GetHeight()-MZM_HEIGHT_TEXT_TOOLBAR, GetWidth(), MZM_HEIGHT_TEXT_TOOLBAR);
		AddUiWin(&m_toolbar);

		GetFullPathFileName(m_fileNameBuffer, m_fileNameBufferLen, L"sgf");
		m_sgfPath = m_fileNameBuffer;

		AnimateWindow(MZ_ANIMTYPE_ZOOM_IN, true); //ӦMStoreҪ�󣬱����д��ڶ���
		Show();

		//���ϴ��˳�ʱ�������ļ������ָ���ʱ�����״̬
		TCHAR* szLastSgfFile = NULL;
		IniReadString(APP_NAME, L"LastQipuFile", &szLastSgfFile, m_mainSettingsFile);
		if(szLastSgfFile && szLastSgfFile[0])
		{
			MzAutoMsgBoxEx(m_hWnd, L"��ԭ�ϴ����״̬...", 0);
			if(LoadFromSGF(szLastSgfFile))
			{
				DWORD lastStoneNo = -1;
				IniReadInt(APP_NAME, L"LastStoneNo.", &lastStoneNo, m_mainSettingsFile);
				if(lastStoneNo != (DWORD)-1)
				{
					m_isInDapuState = (lastStoneNo > 0);
					SetToolBarButtons(m_isInDapuState, lastStoneNo==0);

					m_stoneIndex = -1;
					if(lastStoneNo > 0)
						GoToStone(lastStoneNo - 1);
				}
			}
		}
		if(szLastSgfFile) free(szLastSgfFile);

		//ע�Ტ��ȡ����֪ͨ�¼���Ϣ
		m_usbNotifyMsg = RegisterUsbNotifyMsg();
		RegisterShellMessage(m_hWnd, WM_MZSH_ALL_KEY_EVENT);
		m_allkeyEventMsg = GetShellNotifyMsg_AllKeyEvent();

		return TRUE;
	}

	void OnQuit()
	{
		SaveDapuState();
		//IniWriteInt(APP_NAME, L"Volume", m_soundVolume, m_mainSettingsFile);
		if(m_eventHandle) CloseHandle(m_eventHandle);
	}

	void Quit()
	{
		OnQuit();
		AnimateWindow(MZ_ANIMTYPE_ZOOM_OUT, false); //ӦMStoreҪ�󣬱����д��ڶ���
		PostQuitMessage(0);
	}

	static void SGF_OnProperty(SGFParseContext* pContext, const char* szID, const char* szValue)
	{
		CWeiqiPuMainWnd* pMainWnd = (CWeiqiPuMainWnd*) pContext->pUserData;
		return pMainWnd->sgfOnProperty(pContext, szID, szValue);
	}

	StoneInfo* GetLastStone()
	{
		StoneInfo* pLastStone = NULL;
		if(m_stonesMem.GetDataSize() >= sizeof(StoneInfo))
			pLastStone = (StoneInfo*) m_stonesMem.GetOffsetData(m_stonesMem.GetDataSize() - sizeof(StoneInfo));
		return pLastStone;
	}

	const TCHAR* AppendCharsToStringContainer(const char* s)
	{
		int n = mbstowcs(NULL, s, 0);
		if(n > 0)
		{
			TCHAR* p = (TCHAR*) m_stringContainer.NewMemBlock( (n+1) * sizeof(WCHAR) );
			p[n] = L'\0';
			int nn = mbstowcs(p, s, n);
			__assert(nn == n);
			return p;
		}
		return NULL;
	}

	//-2,-3�ֱ��ʾԤ�úڰ�����, -1��ʾ����
	int getSpecialIndex(StoneColor color)
	{
		switch(color)
		{
		case SC_BLANK: return -1;
		case SC_BLACK: return -2;
		case SC_WHITE: return -3;
		default: return -1;
		}
	}

	//������AB,AWָ����Ԥ������
	void setStaticStones(const char* szValue, StoneColor color)
	{
		if(szValue[0] && szValue[1] && szValue[2]==':') //Compressed point lists, see http://www.red-bean.com/sgf/sgf4.html#stone
		{
			((char*)szValue)[2] = '\0';
			StoneInfo topleft, bottomright;
			if(SetStoneRowCol(topleft, szValue) && SetStoneRowCol(bottomright, szValue+3))
			{
				for(int row = topleft.row; row <= bottomright.row; row++)
				{
					for(int col = topleft.col; col <= bottomright.col; col++)
					{
						m_board[row-1][col-1] = getSpecialIndex(color);
					}
				}
			}
		}
		else
		{
			StoneInfo stone;
			if(SetStoneRowCol(stone, szValue))
				m_board[stone.row-1][stone.col-1] = getSpecialIndex(color);
		}
	}

	void sgfOnProperty(SGFParseContext* pContext, const char* szID, const char* szValue)
	{
		if(pContext->treeIndex != 0) return;

		if(StrEquals(szID, "B"))
		{
			StoneInfo stone;
			stone.color = SC_BLACK;
			stone.stoneIndex = m_stonesMem.GetDataSize() / sizeof(StoneInfo);
			if(SetStoneRowCol(stone, szValue))
				m_board[stone.row-1][stone.col-1] = stone.stoneIndex;
			m_stonesMem.AppendMem(&stone, sizeof(StoneInfo));
		}
		else if(StrEquals(szID, "W"))
		{
			StoneInfo stone;
			stone.color = SC_WHITE;
			stone.stoneIndex = m_stonesMem.GetDataSize() / sizeof(StoneInfo);
			if(SetStoneRowCol(stone, szValue))
				m_board[stone.row-1][stone.col-1] = stone.stoneIndex;
			m_stonesMem.AppendMem(&stone, sizeof(StoneInfo));
		}
		else if(StrEquals(szID, "AB"))
		{
			setStaticStones(szValue, SC_BLACK);
		}
		else if(StrEquals(szID, "AW"))
		{
			setStaticStones(szValue, SC_WHITE);
		}
		else if(StrEquals(szID, "AE"))
		{
			setStaticStones(szValue, SC_BLANK);
		}
		else if(StrEquals(szID, "C"))
		{
			if(strlen(szValue) == 0)
				return;
			StoneInfo* pLastStone = GetLastStone();

			//int wlen = mbstowcs(NULL, szValue, 0);
			int wlen = cncharsToWChar(szValue, NULL, 0);
			WCHAR* old_explain = (pLastStone ?  (WCHAR*)pLastStone->szExplain : m_game_explain);
			WCHAR* new_explain = NULL;

			if(old_explain == NULL)
			{
				new_explain = (WCHAR*) m_stringContainer.NewMemBlock( (wlen+1) * sizeof(WCHAR) );
				//size_t x = mbstowcs(new_explain, szValue, wlen);
				//__assert(x == wlen);
				cncharsToWChar(szValue, new_explain, wlen+1);
				new_explain[wlen] = L'\0';
				m_numOfExplain++;
			}
			else
			{
				int old_wlen = wcslen(old_explain);
				new_explain = (WCHAR*) m_stringContainer.NewMemBlock( (old_wlen + 2 + wlen + 1) * sizeof(WCHAR) );
				memcpy(new_explain, old_explain, (old_wlen) * sizeof(WCHAR));
				memcpy(new_explain + old_wlen, L"\r\n", 2 * sizeof(WCHAR));
				//size_t x = mbstowcs((WCHAR*)new_explain + old_wlen + 2, szValue, wlen);
				//__assert(x == wlen);
				cncharsToWChar(szValue, (WCHAR*)new_explain + old_wlen + 2, wlen+1);
				new_explain[wlen] = L'\0';
			}

			//MzMessageBoxEx(m_hWnd, new_explain, NULL);

			if(pLastStone)
				pLastStone->szExplain = new_explain;
			else
				m_game_explain = new_explain;
		}
		else if(StrEquals(szID, "LB"))
		{
			StoneInfo* pLastStone = GetLastStone();
			__assert(pLastStone);
			if(pLastStone == NULL) return;

			LabelInfo labelinfo;
			labelinfo.col = PointCharToRowOrCol(szValue[0]);
			labelinfo.row = PointCharToRowOrCol(szValue[1]);
			labelinfo.label = AppendCharsToStringContainer(szValue + 3);
			__assert(szValue[2] == ':');

			if(pLastStone->extraInfo == NULL)
				pLastStone->extraInfo = new StoneExtraInfo();
			pLastStone->extraInfo->labelInfos.AppendMem(&labelinfo, sizeof(LabelInfo));
		}
		else if(StrEquals(szID, "EV"))
		{
			m_gameInfo.ev = charsToString(szValue);
		}
		else if(StrEquals(szID, "DT"))
		{
			m_gameInfo.dt = charsToString(szValue);
		}
		else if(StrEquals(szID, "PB"))
		{
			m_gameInfo.pb = charsToString(szValue);
		}
		else if(StrEquals(szID, "PW"))
		{
			m_gameInfo.pw = charsToString(szValue);
		}
		else if(StrEquals(szID, "BR"))
		{
			m_gameInfo.br = charsToString(szValue);
		}
		else if(StrEquals(szID, "WR"))
		{
			m_gameInfo.wr = charsToString(szValue);
		}
		else if(StrEquals(szID, "KM"))
		{
			m_gameInfo.km = charsToString(szValue);
		}
		else if(StrEquals(szID, "RE"))
		{
			m_gameInfo.re = charsToString(szValue);

			//����ʤ����Ϣ�����õ�m_gameInfo.result
			if(StrEquals(szValue, "0") || StrEquals(szValue, "draw"))
			{
				m_gameInfo.result = L"ƽ��";
			}
			else if(StrEquals(szValue, "?") || StrEquals(szValue, "void"))
			{
				m_gameInfo.result = L"ʤ��δ֪";
			}
			else
			{
				if((szValue[0]=='B' || szValue[0]=='W') && szValue[1] == '+')
				{
					bool blackWin = (szValue[0]=='B');
					if(szValue[2] == 'R') //Resign
						m_gameInfo.result = (blackWin ? L"������ʤ" : L"������ʤ");
					else if(szValue[2] == 'T') //Time
						m_gameInfo.result = (blackWin ? L"��ʤ(�Է���ʱ)" : L"��ʤ(�Է���ʱ)");
					else if(szValue[2] == 'F') //Forfeit
						m_gameInfo.result = (blackWin ? L"��ʤ(�Է��и�)" : L"��ʤ(�Է��и�)");
					else if(szValue[2] >= '0' && szValue[2] <= '9')
					{
						m_gameInfo.result = (blackWin ? L"��ʤ" : L"��ʤ");
						char* dotIndex = (char*) strchr(szValue + 2, '.');
						if(dotIndex && *(dotIndex+1) == '5') //ʤNĿ��(n+0.5)
						{
							*dotIndex = '\0';
							m_gameInfo.result = m_gameInfo.result + L" " + charsToString(szValue+2) + L" Ŀ��";
						}
						else
						{
							m_gameInfo.result = m_gameInfo.result + L" " + charsToString(szValue+2) + L" Ŀ";
						}
					}
					else
						m_gameInfo.result = m_gameInfo.re; //δ�ܽ���
				}
				else
					m_gameInfo.result = m_gameInfo.re; //δ�ܽ���
			}
		}
		else if(StrEquals(szID, "GM")) //��������
		{
			m_gameInfo.gm = atoi(szValue);
			if(m_gameInfo.gm != 1)
				MzMessageBoxEx(m_hWnd, L"���棺�޷�ʶ��������ļ���", NULL);
		}
		else if(StrEquals(szID, "PL")) //���Ȱ���?
		{
			if(StrEquals(szValue, "B"))
				m_blackFirst = true;
			else if(StrEquals(szValue, "W"))
				m_blackFirst = false;
		}
	}

	void GetGameInfoString()
	{
		//�ڷ�
		CMzString black = L"����";
		if(m_gameInfo.pb.Length() > 0)
			black = m_gameInfo.pb;
		if(m_gameInfo.br.Length() > 0)
			black = black + L"(" + m_gameInfo.br+ L")"; 
		black = black + L"ִ��";

		//�׷�
		CMzString white = L"����";
		if(m_gameInfo.pw.Length() > 0)
			white = m_gameInfo.pw;
		if(m_gameInfo.wr.Length() > 0)
			white = white + L"(" + m_gameInfo.wr + L")";
		white = white + L"ִ��";

		//�ڷ� VS �׷�
		m_gameInfoString = black + L" VS " + white;

		//�������ƣ�ʱ��
		if(m_gameInfo.ev.Length() > 0)
		{
			m_gameInfoString = m_gameInfoString + L"\r\n" + m_gameInfo.ev;
			if(m_gameInfo.dt.Length() > 0)
				m_gameInfoString = m_gameInfoString + L" [" + m_gameInfo.dt + L"]";
		}

		//�����Ϸ��ռ����ֻ����ʾ�����ı���������Ϣ������ʾ�������·�
	}

	void CleanBoard(bool cleanStaticStones)
	{
		//memset(m_board, 0, sizeof(StoneColor) * 19 * 19);
		for(int i = 0; i < 19; i++)
		{
			for(int j = 0; j < 19; j++)
			{
				if(cleanStaticStones)
				{
					m_board[i][j] = -1;
				}
				else
				{
					if(m_board[i][j] >= 0)
						m_board[i][j] = -1;
				}
			}
		}
	}

	void Unload()
	{
		if(m_stoneCount > 0 && m_killedStones)
		{
			for(int i = 0; i < m_stoneCount; i++)
			{
				if(m_killedStones[i])
					delete m_killedStones[i];
			}
		}
		for(int i = 0; i < m_stoneCount; i++)
		{
			StoneInfo* pStoneInfo = m_pStones + i;
			if(pStoneInfo->extraInfo)
			{
				delete pStoneInfo->extraInfo;
				pStoneInfo->extraInfo = NULL;
			}
		}

		CleanBoard(true);
		m_stonesMem.Empty();
		m_pStones = NULL;
		m_stoneCount = 0;
		m_stringContainer.FreeAll();
		m_current_explain = m_game_explain = NULL;
		m_numOfExplain = 0;
		m_gameInfo.Init();
		m_gameInfoString = L"";
		m_sgfFile = L"";
		m_isInDapuState = false;
		m_stoneIndex = -1;
		m_blackFirst = true;
	}

	bool LoadFromSGF(const TCHAR* szSGFFile)
	{
		char szfile[1024] = {0};
		wcstombs(szfile, szSGFFile, sizeof(szfile));
		const char* sgftext = (const char*) LoadFileData(szfile, malloc, NULL, NULL, 1);
		if(sgftext)
		{
			Unload();
			m_sgfFile = szSGFFile;

			SGFParseContext context;
			initSGFParseContext(&context, NULL, NULL, NULL, NULL, SGF_OnProperty, this);
			parseSGF(&context, sgftext, 0);
			cleanupSGFParseContext(&context);

			m_stoneCount = m_stonesMem.GetDataSize() / sizeof(StoneInfo);
			m_pStones = (StoneInfo*) m_stonesMem.GetData();

			m_killedStonesMem.Empty();
			m_killedStonesMem.AppendZeroMem(m_stoneCount * sizeof(BufferedMem*));
			m_killedStones = (BufferedMem**) m_killedStonesMem.GetData();

			free((void*)sgftext);

			GetGameInfoString();
			m_stoneIndex = m_stoneCount; //��ʾ�վ�״̬���������Ϣ
			InvalidateRect(m_hWnd, NULL, TRUE);
			return true;
		}
		return false;
	}

	//return the y of next line
	static int DrawTextInArea(const TCHAR* szText, HDC hdc, int x, int y, int cx, int cy)
	{
		SIZE lineSize;
		RECT rc;
		TCHAR c;
		const TCHAR* szLine = szText;
		int top = y;
		bool meet_crln = false;
		int n = wcslen(szText);

		for(int i = 0; i < n; i++)
		{
			int crln_charnum = 0;
			c = szText[i];
			meet_crln = (c == L'\r' || c == L'\n');
			if(meet_crln)
			{
				if( (c==L'\r' && szText[i+1]==L'\n') || (c==L'\n' && szText[i+1]==L'\r') ) { i++; crln_charnum++; }
				i++; crln_charnum++;
			}

			GetTextExtentPoint(hdc, szLine, szText + i - szLine + 1, &lineSize);
			if(meet_crln && lineSize.cy == 0)
				GetTextExtentPoint(hdc, L" ", 1, &lineSize);

			if(meet_crln || lineSize.cx > cx)
			{
				rc.left = x; rc.top = y; rc.right = x + cx; rc.bottom = y + lineSize.cy; //the line's rect
				DrawText(hdc, szLine, szText + i - szLine - crln_charnum, &rc, DT_LEFT|DT_TOP|DT_SINGLELINE); //draw this line, not inlude the last char
				y += lineSize.cy;
				szLine = szText+i;
				if(meet_crln) i--;
				if(y >= top + cy) return y; //or return -1 ?
			}
		}

		//draw the last line, if any
		if(szLine < szText + n)
		{
			rc.left = x; rc.top = y; rc.right = x + cx; rc.bottom = y + lineSize.cy; //the line's rect
			DrawText(hdc, szLine, szText + n - szLine, &rc, DT_LEFT|DT_TOP|DT_SINGLELINE); //not inlude the last char
		}

		return (y + lineSize.cy);
	}

	//Ҫ��outӦ������4���ַ��ռ�
	static void FormatStonePos(StoneInfo* pStoneInfo, TCHAR* out)
	{
		if(pStoneInfo->color == SC_BLANK)
		{
			wcscpy(out, L"��Ȩ");
		}
		else
		{
			swprintf(out, L"%c%d", L'A'+pStoneInfo->col-1, pStoneInfo->row);
		}
	}

	void PaintStones(HDC hdc)
	{
		//����ÿһ������
		for(int row = 1; row <= 19; row++)
		{
			for(int col = 1; col <= 19; col++)
			{
				int index = m_board[row-1][col-1];
				if(index >= 0)
				{
					StoneInfo* pStoneInfo = m_pStones + index;
					switch(pStoneInfo->color)
					{
					case SC_BLACK:
						m_skin.DrawBlackStone(hdc, row, col);
						//m_skin.DrawBlackNum(hdc, row, col, index + 1);
						break;
					case SC_WHITE:
						m_skin.DrawWhiteStone(hdc, row, col);
						//m_skin.DrawWhiteNum(hdc, row, col, index + 1);
						break;
					}
				}
				else if(index == -2) //index == -2,-3 ��ʾԤ�����ӣ����sgfOnProperty()�ж�"AB"/"AW"�Ĵ���
				{
					m_skin.DrawBlackStone(hdc, row, col);
				}
				else if(index == -3)
				{
					m_skin.DrawWhiteStone(hdc, row, col);
				}
			}
		}

		//�����������
		if(m_isInDapuState && m_pStones && m_stoneIndex >= 0 && m_stoneIndex < m_stoneCount)
		{
			StoneInfo* pStoneInfo = m_pStones + m_stoneIndex;
			if(pStoneInfo->color == SC_BLACK)
				m_skin.DrawBlackNewStone(hdc, pStoneInfo->row, pStoneInfo->col);
			if(pStoneInfo->color == SC_WHITE)
				m_skin.DrawWhiteNewStone(hdc, pStoneInfo->row, pStoneInfo->col);
		}

		//������ʾ�����Ϣ������֣�������������

		SetBkMode(hdc, TRANSPARENT);
		SetTextColor(hdc, RGB(0,0,0));
		HFONT hFont = CreateSymtemFont(-20);
		HGDIOBJ hOldFont = SelectObject(hdc, hFont);

		//��ʾlabels
		if(m_isInDapuState && m_pStones && m_stoneIndex >= 0 && m_stoneIndex < m_stoneCount)
		{
			StoneInfo* pStoneInfo = m_pStones + m_stoneIndex;
			if(pStoneInfo->extraInfo)
			{
				int labelCount = pStoneInfo->extraInfo->labelInfos.GetDataSize() / sizeof(LabelInfo);
				LabelInfo* pLabelInfos = (LabelInfo*) pStoneInfo->extraInfo->labelInfos.GetData();
				for(int i = 0; i < labelCount; i++)
				{
					LabelInfo* pLabelInfo = pLabelInfos + i;
					int x = -100, y = -100;
					m_skin.GetStoneXY(pLabelInfo->row, pLabelInfo->col, x, y);
					SIZE labelSize;
					GetTextExtentPoint(hdc, pLabelInfo->label, wcslen(pLabelInfo->label), &labelSize);
					x -= labelSize.cx / 2;
					y -= labelSize.cy / 2;
					DrawTextInArea(pLabelInfo->label, hdc, x, y, labelSize.cx, labelSize.cy);
				}
			}
		}

		//�����Ϸ��Ļ�����Ϣ
		if(m_gameInfoString.Length() > 0)
		{
			int x = 5;
			int y = 3;
			int cx = 480 - 2*x;
			int cy = m_skin.GetBoardTopY() - y;
			DrawTextInArea(m_gameInfoString, hdc, x,y, cx,cy);
		}
		else
		{
			DrawTextInArea(M8_WEIQIPU_VERINON L", by liigo", hdc, 10, 15, 480 - 10, 30);
		}

		//���������Ͻ���ʾ��ǰ���ӷ�������ţ��硰��123���򡰰�18(��Ȩ)����
		if(m_isInDapuState && m_pStones && m_stoneIndex >= 0 && m_stoneIndex < m_stoneCount)
		{
			TCHAR currentStoneInfo[64], stonePos[8];
			StoneInfo* pStoneInfo = m_pStones + m_stoneIndex;
			FormatStonePos(pStoneInfo, stonePos);
			swprintf(currentStoneInfo, L"%s%d(%s)", (m_stoneIndex%2==0 ? L"��" : L"��"), m_stoneIndex+1, stonePos);
			RECT rc;
			rc.top = 3; rc.right = 475; rc.left = 0; rc.bottom = 30;
			DrawText(hdc, currentStoneInfo, wcslen(currentStoneInfo), &rc, DT_RIGHT|DT_TOP|DT_SINGLELINE);
		}
		else if(m_sgfFile.Length() > 0)
		{
			RECT rc;
			rc.top = 3; rc.right = 475; rc.left = 0; rc.bottom = 30;
			DrawText(hdc, m_blackFirst ? L"����" : L"����", -1, &rc, DT_RIGHT|DT_TOP|DT_SINGLELINE);
		}

		//�����·��Ľ�˵��Ϣ����������Ϣ
		int x = 5;
		int y = m_skin.GetBoardBottomY() + 2;
		int cx = 480 - 5 - x;
		int cy = 720 - MZM_HEIGHT_TEXT_TOOLBAR - y;
		if(m_stoneIndex >= 0 && m_stoneIndex < m_stoneCount && m_current_explain)
		{
			//��˵��Ϣ
			DrawTextInArea(m_current_explain, hdc, x,y, cx,cy);
		}
		else if(m_sgfFile.Length() > 0 && (m_stoneIndex == -1 || m_stoneIndex == m_stoneCount))
		{
			//����ǰ���վֺ��������·���ʾ���������Ϣ������������Ӯ����˵������SGF�ļ���
			TCHAR info[128];
			wsprintf(info, L"�� %d �֣�%s���� %d ����˵��", m_stoneCount, (TCHAR*)m_gameInfo.result, m_numOfExplain);
			y = DrawTextInArea(info, hdc, x,y, cx,cy);
			if(m_game_explain)
			{
				cy = 720 - MZM_HEIGHT_TEXT_TOOLBAR - y;
				y = DrawTextInArea(m_game_explain, hdc, x,y, cx,cy); //���ȫ�ֵĽ�˵�����µ�һ��֮ǰ�Ľ�˵��
			}
			cy = 720 - MZM_HEIGHT_TEXT_TOOLBAR - y;
			DrawTextInArea(m_sgfFile, hdc, x,y, cx,cy);
		}

		SelectObject(hdc, hOldFont);
		DeleteObject(hFont);
	}


	virtual void  PaintWin (HDC hdc, RECT *prcUpdate=NULL)
	{
		/*
		HDC hMemDC = CreateCompatibleDC(hdc);
		HBITMAP hMemBmp = CreateCompatibleBitmap(hdc, 480, 720);
		HBITMAP hOldBmp = (HBITMAP) SelectObject(hMemDC, hMemBmp);
		*/

		m_skin.DrawBackground(hdc);
		PaintStones(hdc);

		CMzWndEx::PaintWin(hdc, prcUpdate);

		/*
		BitBlt(hdc, 0, 0, 480, 720, hMemDC, 0, 0, SRCCOPY);
		SelectObject(hMemDC, hOldBmp);
		DeleteObject(hMemBmp);
		DeleteDC(hMemDC);
		*/
	}

	virtual void OnLButtonDown(UINT fwKeys, int xPos, int yPos)
	{
		CMzWndEx::OnLButtonDown(fwKeys, xPos, yPos);
		m_lbuttonDownX = xPos;
		m_lbuttonDownY = yPos;
	}

	virtual void OnLButtonUp(UINT fwKeys, int xPos, int yPos)
	{
		CMzWndEx::OnLButtonUp(fwKeys, xPos, yPos);

		/*
		if(xPos != m_lbuttonDownX || yPos != m_lbuttonDownY) return;
		if(yPos >= 720 - MZM_HEIGHT_TEXT_TOOLBAR) return;

		if(m_isInDapuState && yPos > m_skin.GetBoardBottomY())
		{
			//�û��������·��հ���(��˵��)���, ������൱�ڰ������, ���ұ��൱�ڰ������
			if(xPos < 240)
				NextOrPrevStone(true);
			else
				NextOrPrevStone(false);
			return;
		}
		*/

		return; 

		/*
		//�Ȳ���������������
		int row, col;
		if(m_skin.GetStoneRowColFromXY(xPos, yPos, row, col))
		{
			static bool s_isBlack = true;
			bool drawStoneOK = false;

			if(s_isBlack)
				drawStoneOK = m_skin.DrawBlackStone(GetDC(m_hWnd), row, col);
			else
				drawStoneOK = m_skin.DrawWhiteStone(GetDC(m_hWnd), row, col);

			if(drawStoneOK)
			{
				m_stoneCount++;
				m_stoneIndex++;
				StoneInfo& stone = m_pStones[m_stoneIndex];
				stone.row = row;
				stone.col = col;
				stone.color = (s_isBlack ? SC_BLACK : SC_WHITE);

				s_isBlack = !s_isBlack;
			}
		}
		*/
	}

	bool NextStone()
	{
		if(m_stoneIndex < m_stoneCount - 1)
		{
			m_stoneIndex++;
			StoneInfo& stone = m_pStones[m_stoneIndex];
			m_board[stone.row-1][stone.col-1] = m_stoneIndex;
			if(stone.color != SC_BLANK) //��������Ȩ
				processLiving(stone.row, stone.col);
			m_current_explain = (TCHAR*) stone.szExplain;
			return true;
		}
		else if(m_stoneIndex == m_stoneCount - 1)
		{
			//�վ�ʱ��ʾ�����Ϣ
			m_stoneIndex++;
			m_current_explain = NULL;
			return true; //��Ҫ������������û����
		}
		return false;
	}

	void NextStones(int num)
	{
		if(num == 0)
			return;
		while(num-- > 0)
			NextStone();
		if(m_SoundOn)
			m_skin.PlayLuoziSound();
		Invalidate();
	}

	bool PrevStone()
	{
		if(m_stoneIndex >= 0)
		{
			m_stoneIndex--;
			StoneInfo& stone = m_pStones[m_stoneIndex+1];
			m_board[stone.row-1][stone.col-1] = -1;
			//����Ѻ�һ���Ե�������ʾ����
			BufferedMem* pKilledStones = m_killedStones[m_stoneIndex+1];
			if(pKilledStones)
			{
				int* pBoard = &m_board[0][0];
				StoneIndexInfo* pIndex = (StoneIndexInfo*) pKilledStones->GetData();
				for(int i = 0, n = pKilledStones->GetDataSize()/sizeof(StoneIndexInfo); i < n; i++)
				{
					pBoard[pIndex[i].posIndex] = pIndex[i].stoneIndex;
				}
			}
			m_current_explain = (m_stoneIndex >= 0 ? (TCHAR*) m_pStones[m_stoneIndex].szExplain : NULL);
			return true;
		}
		return false;
	}

	void PrevStones(int num)
	{
		if(num == 0)
			return;
		while(num-- > 0)
			PrevStone();
		Invalidate();
	}

	static void AppendPopupMenuItem(CPopupMenu& menu, TCHAR* szText, int ID, int color = MZC_BUTTON_PELLUCID)
	{
		struct PopupMenuItemProp item;		
		item.str = szText;
		item.itemRetID = ID;
		item.itemCr = color;
		menu.AddItem(item);
	}

	bool NextOrPrevStone(bool isRightButton)
	{
		if((isRightButton && m_NextStoneOnRight) || (!isRightButton && !m_NextStoneOnRight))
		{
			if(NextStone())
			{
				InvalidateRect(m_hWnd, NULL, true);
				if(m_SoundOn)
					m_skin.PlayLuoziSound();
				return true;
			}
			else
			{
				MzAutoMsgBoxEx(m_hWnd, L"����ѽ���", 2000);
			}
		}
		else
		{
			if(PrevStone())
			{
				InvalidateRect(m_hWnd, NULL, true);
				return true;
			}
			else
			{
				MzAutoMsgBoxEx(m_hWnd, L"�������׿�ͷ", 2000);
			}
		}
		return false;
	}

	bool GoToStone(int stoneIndex)
	{
		CleanBoard(false);
		m_stoneIndex = -1;
		NextStones(stoneIndex + 1);
		Invalidate();
		return true;
	}

	void SetToolBarButtons(bool bInDapuState, bool hasBeginDapu = false)
	{
		if(bInDapuState)
		{
			m_toolbar.SetButton(0, true, true, m_NextStoneOnRight ? L"��һ��" : L"��һ��");
			m_toolbar.SetButton(1, true, true, L"�˵�");
			m_toolbar.SetButton(2, true, true, m_NextStoneOnRight ? L"��һ��" : L"��һ��");
		}
		else
		{
			m_toolbar.SetButton(0, true, true, L"�˵�");
			if(hasBeginDapu)
				m_toolbar.SetButton(1, true, true, L"��ʼ����");
			else
				m_toolbar.SetButton(1, false, false, L"");
			m_toolbar.SetButton(2, true, true, L"������");
		}
	}

	// override the MZFC command handler
	virtual void OnMzCommand(WPARAM wParam, LPARAM lParam)
	{
		WORD id = LOWORD(wParam);
		WORD notifyCode = HIWORD(wParam);

		switch(id)
		{
		case MY_IDC_TOOLBAR:
			{
				int index = lParam;
				//���ݴ���״̬���, ���ò�ͬ�Ĺ�������ť�Ͳ˵�
				if(m_isInDapuState)
				{
					//����״̬��
					if(index == 0)
					{
						NextOrPrevStone(false);
					}
					else if(index == 2)
					{
						NextOrPrevStone(true);
					}
					else if(index == 1)
					{
						//���������˵�
						const int MENU_STOP_DAPU = 1, MENU_CANCEL = 2, MENU_SETTING = 3, MENU_PREV10 = 5, MENU_NEXT10 = 6;
						CPopupMenu menu;
						//�����������ʾ�����˳�����෴��, ���ȼ������ʾ��������,��������ʾ��������, �е�ְɺٺ�
						AppendPopupMenuItem(menu, L"ȡ��", MENU_CANCEL);
						AppendPopupMenuItem(menu, L"����", MENU_SETTING);
						AppendPopupMenuItem(menu, L"��ʮ��", MENU_NEXT10);
						AppendPopupMenuItem(menu, L"ǰʮ��", MENU_PREV10);
						AppendPopupMenuItem(menu, L"ֹͣ����", MENU_STOP_DAPU);
						int height = menu.GetHeight(); //POPUP_MENUE_ITEM_HEIGHT * 2.5;
						menu.Create(0, 720 - height, 480,  height,m_hWnd, 0, WS_POPUP);
						//menu.Create(120, 720 - MZM_HEIGHT_TEXT_TOOLBAR - height, 240,  height,m_hWnd, 0, WS_POPUP);
						int nID = menu.DoModal();
						if(nID == MENU_STOP_DAPU)
						{
							m_isInDapuState = false;
							m_stoneIndex = -1;
							SetToolBarButtons(false);
							Unload();
							InvalidateRect(m_hWnd, NULL, true);
						}
						else if(nID == MENU_SETTING)
						{
							CSettingWnd* pSettingWnd = new CSettingWnd(m_mainSettingsFile, &m_SoundOn, &m_NextStoneOnRight, 
																		m_skinsPath, m_skinPath, m_skin.GetName(), m_skin, &m_toolbar);
							pSettingWnd->CreateWnd(m_hWnd);
						}
						else if(nID == MENU_PREV10)
						{
							PrevStones(10);
						}
						else if(nID == MENU_NEXT10)
						{
							NextStones(10);
						}
					}
				}
				else
				{
					//�Ǵ���״̬��
					if(index == 0)
					{
						//�������˵�
						const int MENU_SETTING = 1, MENU_EXIT = 2, MENU_CANCEL = 3;
						CPopupMenu menu;
						//�����������ʾ�����˳�����෴��, ���ȼ������ʾ��������,��������ʾ��������, �е�ְɺٺ�
						AppendPopupMenuItem(menu, L"ȡ��", MENU_CANCEL);
						AppendPopupMenuItem(menu, L"�˳�", MENU_EXIT);
						AppendPopupMenuItem(menu, L"����", MENU_SETTING);
						int height = menu.GetHeight();
						menu.Create(0, 720 - height, 480,  height,m_hWnd, 0, WS_POPUP);
						int nID = menu.DoModal();
						if(nID == MENU_EXIT)
						{
							//if(MzMessageBoxEx(m_hWnd, L"���Ҫ�˳�M8Χ������", L"�˳�", MB_YESNO, 1) == 1) //if YES
							{
								//ShowMzTopBar();
								//MzGetApp()->EnableNeverQuit(false);
								Quit();
							}
						}
						else if(nID == MENU_SETTING)
						{
							CSettingWnd* pSettingWnd = new CSettingWnd(m_mainSettingsFile, &m_SoundOn, &m_NextStoneOnRight, 
																		m_skinsPath, m_skinPath, m_skin.GetName(), m_skin, NULL);
							pSettingWnd->CreateWnd(m_hWnd);
						}
						break;
					}
					if(index == 1 && m_toolbar.IsButtonShow(1))
					{
						//��ʼ����: �������״̬, ������������ť, ��ʾ���̵�һ��
						m_isInDapuState = true;
						SetToolBarButtons(true);

						CleanBoard(false);
						m_stoneIndex = -1;
						if(NextStone())
						{
							if(m_SoundOn)
								m_skin.PlayLuoziSound();
							InvalidateRect(m_hWnd, NULL, TRUE);
						}
						break;
					}
					if(index == 2)
					{
						//ѡ�񲢼��������ļ�
						OpenQipuFile();
						break;
					}
				}
			}
			break;
		}
	}

	virtual LRESULT MzDefWndProc(UINT message, WPARAM wParam, LPARAM lParam)
	{
		//can not uses switch(message), error C2051: case expression not constant

		if( message == m_usbNotifyMsg) //ӦMStoreҪ��USB����ʱ��Ĭ�˳�����
		{
			if(wParam == USB_MASSSTORAGE_ATTACH) 
			{
				Quit();
			}
		}
		else if(message == m_allkeyEventMsg) //ӦMStoreҪ����������������
		{
			int key = LOWORD(wParam); //WPARAM_KEY_EVENT_***
			if(key == WPARAM_KEY_EVENT_CLICK_VOLUP)
			{
				m_soundVolume += 7; //�밴��������ʱ��ϵͳ�����仯���ȱ��ֻ���һ��
				if(m_soundVolume > 100) m_soundVolume = 100;
				m_skin.SetSoundVolume(m_soundVolume);
			}
			else if(key == WPARAM_KEY_EVENT_CLICK_VOLDOWN)
			{
				m_soundVolume -= 7;
				if(m_soundVolume < 0) m_soundVolume = 0;
				m_skin.SetSoundVolume(m_soundVolume);
			}
		}

		/*
		if(message == WM_ACTIVATE)
		{
			//�����ʱ������Ϣ������֤ʼ��ȫ��
			WORD activeFlag = LOWORD(wParam);
			if(activeFlag == WA_ACTIVE || activeFlag == WA_CLICKACTIVE)
				HideMzTopBar();
		}
		*/
		return CMzWndEx::MzDefWndProc(message, wParam, lParam);
	}

	virtual int OnShellHomeKey(UINT message, WPARAM wParam, LPARAM lParam)
	{
		OnQuit();
		return SHK_RET_DEFAULT; //quit
	}

	//�˳�ǰ��¼������Ϣ���Ա��´δ򿪳���ʱ�ָ��Դ�ʱ�����״̬��������
	void SaveDapuState()
	{
		IniWriteString(APP_NAME, L"LastQipuFile", m_sgfFile, m_mainSettingsFile);
		IniWriteInt(APP_NAME, L"LastStoneNo.", (m_isInDapuState ? m_stoneIndex + 1 : 0), m_mainSettingsFile);
	}

	void OpenQipuFile()
	{
		IMzSelect *pSelect = NULL; 
		IFileBrowser *pFile = NULL;                      
		CoInitializeEx(NULL, COINIT_MULTITHREADED );
		if ( SUCCEEDED( CoCreateInstance( CLSID_FileBrowser, NULL,CLSCTX_INPROC_SERVER ,IID_MZ_FileBrowser,(void **)&pFile ) ) )
		{
			if( SUCCEEDED( pFile->QueryInterface( IID_MZ_Select, (void**)&pSelect ) ) )
			{
				TCHAR file[ MAX_PATH ] = { 0 };
				pFile->SetParentWnd( m_hWnd );
				pFile->SetOpenDirectoryPath( m_sgfPath); //��������ô˺�����Ĭ��Ϊ��Ŀ¼
				pFile->SetExtFilter( L"*.*" );
				pFile->SetOpenDocumentType(DOCUMENT_SELECT_SINGLE_FILE); //Ӧ�ø�����������ĵ��򿪷�ʽ������
				//ShowMzTopBar(); //�������ʱ��ʾ������������,��һ���հ׺��ѿ���
				if( pSelect->Invoke() ) 
				{//��Ӧ�ø����Լ������ȡ�ĵ��ķ���ֵ
					//MzMessageBoxEx(m_hWnd, pFile->GetSelectedFileName(), NULL);
					LoadFromSGF(pFile->GetSelectedFileName());
					m_isInDapuState = false;
					m_toolbar.SetButton(1, true, true, L"��ʼ����");
				}
				//HideMzTopBar(); //�ָ�ȫ��
				pSelect->Release();
			}
			pFile->Release();
		}
		CoUninitialize();
		::InvalidateRect( m_hWnd, NULL, FALSE );
		::UpdateWindow( m_hWnd );
	}

	//���������Ӷ��ܱ߶Է��ӵ�����Ӱ��
	void processLiving(int row, int col)
	{
		StoneColor color = GetBoardStoneColor(row, col);
		if(color == SC_BLANK)
			return; //��������Ȩ����
		StoneColor otherColor = (color == SC_BLACK ? SC_WHITE : SC_BLACK);

		if(m_killedStones[m_stoneIndex] == NULL)
			m_killedStones[m_stoneIndex] = new BufferedMem(20);
		BufferedMem* pKilledStones = m_killedStones[m_stoneIndex];
		pKilledStones->Empty();

		//����ܱ��ǶԷ�����, �������������, ���˵��õ�
		BufferedMem stoneIndexList;
		if(row>1 && GetBoardStoneColor(row-1,col)==otherColor && checkLiving(row-1,col,color,&stoneIndexList)==false)
			processDeadStones(&stoneIndexList);
		stoneIndexList.Empty();
		if(row<19 && GetBoardStoneColor(row+1,col)==otherColor && checkLiving(row+1,col,color,&stoneIndexList)==false)
			processDeadStones(&stoneIndexList);
		stoneIndexList.Empty();
		if(col>1 && GetBoardStoneColor(row,col-1)==otherColor && checkLiving(row,col-1,color,&stoneIndexList)==false)
			processDeadStones(&stoneIndexList);
		stoneIndexList.Empty();
		if(col<19 && GetBoardStoneColor(row,col+1)==otherColor && checkLiving(row,col+1,color,&stoneIndexList)==false)
			processDeadStones(&stoneIndexList);
		InvalidateRect(m_hWnd, NULL, true);
	}

	StoneInfo* GetBoardStoneInfo(int row, int col)
	{
		int index = m_board[row-1][col-1];
		if(index == -1)
			return NULL;
		else
			return m_pStones + index;
	}

	StoneColor GetBoardStoneColor(int row, int col)
	{
		StoneInfo* pStoneInfo = GetBoardStoneInfo(row, col);
		return (pStoneInfo ? pStoneInfo->color : SC_BLANK);
	}

	//���row/col���ڵ��ӵ�����, colorΪ��һ���ӵ���ɫ. 
	//���� 1 ��ʾ����(û��), ���� 0 ��ʾ����, ���� -1 ��ʾ����δ��
	int checkLiving(int row, int col, StoneColor color, BufferedMem* pStoneIndexList)
	{
		StoneInfo* pStoneInfo = GetBoardStoneInfo(row, col);
		if(pStoneInfo == NULL || pStoneInfo->color == SC_BLANK) //����, ���Ի���
			return 1;
		if(pStoneInfo->color == color) //���ǶԷ�����
			return -1;

		int index = RowColToIndex(row, col);
		StoneIndexInfo* pIndex = (StoneIndexInfo*) pStoneIndexList->GetData();
		for(int i = 0, n = pStoneIndexList->GetDataSize()/sizeof(StoneIndexInfo); i < n; i++)
		{
			if(pIndex[i].posIndex == index)
				return -1; //�Ѿ������������
		}

		StoneIndexInfo indexInfo;
		indexInfo.posIndex = index;
		indexInfo.stoneIndex = pStoneInfo->stoneIndex;
		pStoneIndexList->AppendMem(&indexInfo, sizeof(indexInfo));

		//�ݹ����ܱ߼�������, ֻҪ������һ������˵������
		//��������ظ��������, ��Ҫ�Ż�
		if(row > 1 && GetBoardStoneColor(row-1,col) != color && checkLiving(row-1, col, color, pStoneIndexList)==1)
			return 1;
		if(row < 19 && GetBoardStoneColor(row+1,col) != color && checkLiving(row+1, col, color, pStoneIndexList)==1)
			return 1;
		if(col > 1 && GetBoardStoneColor(row,col-1) != color && checkLiving(row, col-1, color, pStoneIndexList)==1)
			return 1;
		if(col < 19 && GetBoardStoneColor(row,col+1) != color && checkLiving(row, col+1, color, pStoneIndexList)==1)
			return 1;

		return 0;
	}

	void processDeadStones(BufferedMem* deadStoneIndexList)
	{
		//����������������
		StoneIndexInfo* pIndex = (StoneIndexInfo*) deadStoneIndexList->GetData();
		int* pBoard = &m_board[0][0];
		for(int i = 0, n = deadStoneIndexList->GetDataSize()/sizeof(StoneIndexInfo); i < n; i++)
			pBoard[pIndex[i].posIndex] = -1; 

		//���������Ӽ�¼����, �Ա���ǰ����ʱ�ָ�
		if(deadStoneIndexList->GetDataSize() > 0)
		{
			BufferedMem* pKilledStones = m_killedStones[m_stoneIndex];
			assert(pKilledStones);
			pKilledStones->AppendMem(deadStoneIndexList->GetData(), deadStoneIndexList->GetDataSize());//�˴����ܻ����ظ�,������ν
		}
	}
};

MZ_IMPLEMENT_DYNAMIC(CWeiqiPuMainWnd)

// Application class derived from CMzApp
class CWeiqiPuApp: public CMzApp
{
public:
	// The main window of the app.
	CWeiqiPuMainWnd m_MainWnd;

	// Initialization of the application
	virtual BOOL Init()
	{
		// Init the COM relative library.
		CoInitializeEx(0, COINIT_MULTITHREADED);
		//HideMzTopBar();

		// Create the main window
		//RECT rcWork = { 0, 0, 480, 720 };
		RECT rcWork = MzGetWorkArea(); /* MzGetWorkArea()���ص�����ʼ�ղ����������ź���,��ʹ���ѱ�����  */
		m_MainWnd.Create(rcWork.left,rcWork.top,RECT_WIDTH(rcWork),RECT_HEIGHT(rcWork), 0, 0, 0);
		m_MainWnd.Show();

		//this->EnableNeverQuit(true);

		// return TRUE means init success.
		return TRUE;
	}
};

// The global variable of the application.
CWeiqiPuApp theApp;
