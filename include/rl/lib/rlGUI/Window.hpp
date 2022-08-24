#pragma once
#ifndef ROBINLE_LIB_GUI_WINDOW
#define ROBINLE_LIB_GUI_WINDOW





#include "IControl.hpp"
#include "OpenGL.hpp"

#include <Windows.h>
#include <gl/GL.h>

#pragma comment(lib, "Opengl32.lib")

#include <map>
#include <string>



namespace rl
{
	namespace GUI
	{

		/// <summary>
		/// A <c>enum class</c> representation of the <c>DROPEFFECT</c> constants.
		/// </summary>
		enum class DropEffect
		{
			/// <summary>
			/// Window cannot accept the data.
			/// </summary>
			None,
			/// <summary>
			/// Drop results in a copy. The original data is untouched by the drag source.
			/// </summary>
			Copy,
			/// <summary>
			/// Drag source should remove the data.
			/// </summary>
			Move,
			/// <summary>
			/// Drag source should create a link to the original data.
			/// </summary>
			Link
		};

		class Window : public IControl
		{
		public: // methods

			Window(const wchar_t* szClassName, IControl* pOwner, Window* pParent,
				int iLeft, int iTop, unsigned iWidth, unsigned iHeight, Color clBackground);
			Window(const Window& other) = delete;
			virtual ~Window() = default;

			void initialize();

			virtual void repaint() override;

			const std::wstring& getClassName() const { return m_sClassName; }
			auto getParentWindow() const { return m_pParentWindow; }
			const auto& getChildWindows() const { return m_oChildWindows; }
			auto& getChildWindows() { return m_oChildWindows; }
			auto getHandle() const { return m_hWnd; }
			auto getHDC() const { return m_hDC; }
			const auto& getClientRect() const { return m_oClientRect; }

			bool getClosesApp() const { return m_bWinClosesApp; }
			void setClosesApp(bool bWindowClosesApp) { m_bWinClosesApp = bWindowClosesApp; }

			void show();
			void hide();
			bool isVisible() const { return m_bVisible; }

			const wchar_t* getTitle() const { return m_sTitle.c_str(); }
			void setTitle(const wchar_t* szTitle);
			void setTitleASCII(const char* szTitle);

			bool getAcceptsFiles() const { return m_bAcceptsFiles; }
			void setAcceptsFiles(bool bAcceptsFiles);


		protected: // methods

			virtual bool onMessage(LRESULT& result, UINT uMsg, WPARAM wParam, LPARAM lParam)
			{
				return false;
			}

			virtual void onGainFocus() {}
			virtual void onLoseFocus() {}
			virtual void onResized(unsigned iWidth, unsigned iHeight) {}

			virtual void onFileDragEnter() {}
			virtual void onFileDrag(int iX, int iY, DropEffect& eEffect,
				DWORD grfKeyState) {}
			virtual void onFileDragLeave() {}
			virtual void onFileDrop(int32_t iX, int32_t iY, DWORD grfKeyState) {}

			const auto& getDragDropFilenames() const { return m_oDropTarget.getFilenames(); }

			void checkInitialized();


		private: // types

			class DropTarget : public IDropTarget
			{
			public: // methods

				DropTarget(Window& oWindow);
				~DropTarget();

				void initialize();

				const std::vector<std::wstring>& getFilenames() const { return m_oFilenames; }


			protected: // IUnknown methods

				STDMETHOD(QueryInterface)(REFIID riid, void** ppvObj);
				STDMETHOD_(ULONG, AddRef)();
				STDMETHOD_(ULONG, Release)();


			protected: // IDropTarget methods

				STDMETHOD(DragEnter)(IDataObject* pDataObj, DWORD grfKeyState, POINTL pt,
					DWORD* pdwEffect);
				STDMETHOD(DragOver)(DWORD grfKeyState, POINTL pt, LPDWORD pdwEffect);
				STDMETHOD(Drop)(LPDATAOBJECT pDataObj, DWORD grfKeyState, POINTL pt,
					LPDWORD pdwEffect);
				STDMETHOD(DragLeave)();



			private: // variables

				Window& m_oWindow;
				ULONG m_iRefCount = 0;
				std::vector<std::wstring> m_oFilenames;
			};

			friend class DropTarget;


		private: // methods

			LRESULT internalWindowProc(UINT uMsg, WPARAM wParam, LPARAM lParam);


		private: // variables

			const std::wstring m_sClassName;
			Window* const m_pParentWindow;
			bool m_bWinClosesApp = false;
			bool m_bAcceptsFiles = false;
			DropTarget m_oDropTarget = DropTarget(*this);
			RECT m_oClientRect{}; // todo: fill, update

			std::vector<Window*> m_oChildWindows;

			HWND m_hWnd = NULL;
			HDC m_hDC = NULL;

			bool m_bVisible = false;
			bool m_bFocused = false;
			std::wstring m_sTitle;
			bool m_bInitialized = false;

			OpenGL* m_pOpenGL;


		private: // static methods

			static LRESULT CALLBACK GlobalWndProc(HWND hWnd,
				UINT uMsg, WPARAM wParam, LPARAM lParam);


		private: // static variables

			static std::map<HWND, Window*> s_oWindows;

		};

	}
}





#endif // ROBINLE_LIB_GUI_WINDOW