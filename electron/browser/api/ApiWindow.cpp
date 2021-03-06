﻿
#include <node_buffer.h>
#include "common/OptionsSwitches.h"
#include "common/NodeRegisterHelp.h"
#include "browser/api/ApiWebContents.h"
#include "browser/api/WindowList.h"
#include "common/ThreadCall.h"
#include "common/StringUtil.h"
#include "common/NodeBinding.h"
#include "common/api/event_emitter.h"
#include "common/IdLiveDetect.h"
#include "wke.h"
#include "gin/per_isolate_data.h"
#include "gin/object_template_builder.h"

using namespace v8;
using namespace node;

namespace atom {

class Window : public mate::EventEmitter<Window> {
public:
    explicit Window(v8::Isolate* isolate, v8::Local<v8::Object> wrapper) {
        gin::Wrappable<Window>::InitWith(isolate, wrapper);
        m_webContents = nullptr;
        m_state = WindowUninited;
        m_hWnd = nullptr;
        m_memoryBMP = nullptr;
        m_memoryDC = nullptr;
        m_isLayerWindow = false;
        m_clientRect.left = 0;
        m_clientRect.top = 0;
        m_clientRect.right = 0;
        m_clientRect.bottom = 0;
        ::InitializeCriticalSection(&m_memoryCanvasLock);
        m_id = IdLiveDetect::get()->constructed();
    }

    ~Window() {
        DebugBreak();

        if (m_nodeBinding)
            delete m_nodeBinding;

        if (m_memoryBMP)
            ::DeleteObject(m_memoryBMP);
        if (m_memoryDC)
            ::DeleteDC(m_memoryDC);

        //ThreadCall::callUiThreadSync([this] {
            //delete data->m_webContents;
            ::SendMessage(this->m_hWnd, WM_CLOSE, 0, 0);
        //});
        WindowList::GetInstance()->RemoveWindow(this);

        IdLiveDetect::get()->deconstructed(m_id);

        ::DeleteCriticalSection(&m_memoryCanvasLock);
    }

    static void init(Local<Object> target, Environment* env) {
        Isolate* isolate = env->isolate();
        gin::PerIsolateData* perIsolateData = new gin::PerIsolateData(isolate, nullptr);

        Local<FunctionTemplate> prototype = FunctionTemplate::New(isolate, newFunction); // Function模板
        
        prototype->SetClassName(String::NewFromUtf8(isolate, "BrowserWindow"));
        gin::ObjectTemplateBuilder builder(isolate, prototype->InstanceTemplate());
        builder.SetMethod("webContents", &Window::getWebContentsApi);
        builder.SetMethod("close", &Window::closeApi);
        builder.SetMethod("focus", &Window::focusApi);
        builder.SetMethod("blur", &Window::blurApi);
        builder.SetMethod("isFocused", &Window::isFocusedApi);
        builder.SetMethod("show", &Window::showApi);
        builder.SetMethod("showInactive", &Window::showInactiveApi);
        builder.SetMethod("hide", &Window::hideApi);
        builder.SetMethod("isVisible", &Window::isVisibleApi);
        builder.SetMethod("isEnabled", &Window::isEnabledApi);
        builder.SetMethod("maximize", &Window::maximizeApi);
        builder.SetMethod("unmaximize", &Window::unmaximizeApi);
        builder.SetMethod("isMaximized", &Window::isMaximizedApi);
        builder.SetMethod("minimize", &Window::minimizeApi);
        builder.SetMethod("restore", &Window::restoreApi);
        builder.SetMethod("isMinimized", &Window::isMinimizedApi);
        builder.SetMethod("setFullScreen", &Window::setFullScreenApi);
        builder.SetMethod("isFullScreen", &Window::isFullScreenApi);
        builder.SetMethod("setAspectRatio", &Window::nullFunction);
        builder.SetMethod("previewFile", &Window::nullFunction);
        builder.SetMethod("closeFilePreview", &Window::nullFunction);
        builder.SetMethod("setParentWindow", &Window::nullFunction);
        builder.SetMethod("getParentWindow", &Window::nullFunction);
        builder.SetMethod("getChildWindows", &Window::nullFunction);
        builder.SetMethod("isModal", &Window::isModalApi);
        builder.SetMethod("getNativeWindowHandle", &Window::getNativeWindowHandleApi);
        builder.SetMethod("getBounds", &Window::getBoundsApi);
        builder.SetMethod("setBounds", &Window::setBoundsApi);
        builder.SetMethod("getSize", &Window::getSizeApi);
        builder.SetMethod("setSize", &Window::setSizeApi);
        builder.SetMethod("getContentBounds", &Window::getContentBoundsApi);
        builder.SetMethod("setContentBounds", &Window::setContentBoundsApi);
        builder.SetMethod("getContentSize", &Window::getContentSizeApi);
        builder.SetMethod("setContentSize", &Window::setContentSizeApi);
        builder.SetMethod("setMinimumSize", &Window::setMinimumSizeApi);
        builder.SetMethod("getMinimumSize", &Window::getMinimumSizeApi);
        builder.SetMethod("setMaximumSize", &Window::setMaximumSizeApi);
        builder.SetMethod("getMaximumSize", &Window::getMaximumSizeApi);
        builder.SetMethod("setSheetOffset", &Window::nullFunction);
        builder.SetMethod("setResizable", &Window::setResizableApi);
        builder.SetMethod("isResizable", &Window::isResizableApi);
        builder.SetMethod("setMovable", &Window::setMovableApi);
        builder.SetMethod("isMovable", &Window::isMovableApi);
        builder.SetMethod("setMinimizable", &Window::setMinimizableApi);
        builder.SetMethod("isMinimizable", &Window::isMinimizableApi);
        builder.SetMethod("isMaximizable", &Window::isMaximizableApi);
        builder.SetMethod("setFullScreenable", &Window::setFullScreenableApi);
        builder.SetMethod("isFullScreenable", &Window::isFullScreenableApi);
        builder.SetMethod("setClosable", &Window::setClosableApi);
        builder.SetMethod("isClosable", &Window::isClosableApi);
        builder.SetMethod("setAlwaysOnTop", &Window::setAlwaysOnTopApi);
        builder.SetMethod("isAlwaysOnTop", &Window::isAlwaysOnTopApi);
        builder.SetMethod("center", &Window::centerApi);
        builder.SetMethod("setPosition", &Window::setPositionApi);
        builder.SetMethod("getPosition", &Window::getPositionApi);
        builder.SetMethod("setTitle", &Window::setTitleApi);
        builder.SetMethod("getTitle", &Window::getTitleApi);
        builder.SetMethod("flashFrame", &Window::flashFrameApi);
        builder.SetMethod("setSkipTaskbar", &Window::setSkipTaskbarApi);
        builder.SetMethod("setKiosk", &Window::nullFunction);
        builder.SetMethod("isKiosk", &Window::nullFunction);
        builder.SetMethod("setBackgroundColor", &Window::setBackgroundColorApi);
        builder.SetMethod("setHasShadow", &Window::nullFunction);
        builder.SetMethod("hasShadow", &Window::nullFunction);
        builder.SetMethod("setRepresentedFilename", &Window::nullFunction);
        builder.SetMethod("getRepresentedFilename", &Window::nullFunction);
        builder.SetMethod("setDocumentEdited", &Window::setDocumentEditedApi);
        builder.SetMethod("isDocumentEdited", &Window::isDocumentEditedApi);
        builder.SetMethod("setIgnoreMouseEvents", &Window::setIgnoreMouseEventsApi);
        builder.SetMethod("setContentProtection", &Window::setContentProtectionApi);
        builder.SetMethod("setFocusable", &Window::setFocusableApi);
        builder.SetMethod("focusOnWebView", &Window::focusOnWebViewApi);
        builder.SetMethod("blurWebView", &Window::blurApi);
        builder.SetMethod("isWebViewFocused", &Window::isWebViewFocusedApi);
        builder.SetMethod("setOverlayIcon", &Window::setOverlayIconApi);
        builder.SetMethod("setThumbarButtons", &Window::setThumbarButtonsApi);
        builder.SetMethod("setMenu", &Window::setMenuApi);
        builder.SetMethod("setAutoHideMenuBar", &Window::setAutoHideMenuBarApi);
        builder.SetMethod("isMenuBarAutoHide", &Window::isMenuBarAutoHideApi);
        builder.SetMethod("setMenuBarVisibility", &Window::setMenuBarVisibilityApi);
        builder.SetMethod("isMenuBarVisible", &Window::isMenuBarVisibleApi);
        builder.SetMethod("setVisibleOnAllWorkspaces", &Window::setVisibleOnAllWorkspacesApi);
        builder.SetMethod("isVisibleOnAllWorkspaces", &Window::isVisibleOnAllWorkspacesApi);
        builder.SetMethod("setVibrancy", &Window::nullFunction);
        builder.SetMethod("hookWindowMessage", &Window::hookWindowMessageApi);
        builder.SetMethod("isWindowMessageHooked", &Window::isWindowMessageHookedApi);
        builder.SetMethod("unhookWindowMessage", &Window::unhookWindowMessageApi);
        builder.SetMethod("unhookAllWindowMessages", &Window::unhookAllWindowMessagesApi);
        builder.SetMethod("setThumbnailClip", &Window::setThumbnailClipApi);
        builder.SetMethod("setThumbnailToolTip", &Window::setThumbnailToolTipApi);
        builder.SetMethod("setAppDetails", &Window::setAppDetailsApi);
        builder.SetMethod("setIcon", &Window::setIconApi);
        //NODE_SET_PROTOTYPE_METHOD(prototype, &Window::"id", &Window::nullFunction);
        builder.SetMethod("getWebContents", &Window::getWebContentsApi);

        // 设置constructor
        constructor.Reset(isolate, prototype->GetFunction());
        // export `BrowserWindow`
        target->Set(String::NewFromUtf8(isolate, "BrowserWindow"), prototype->GetFunction());
    }

//     static void staticOnPaintUpdated(wkeWebView webView, Window* win, const HDC hdc, int x, int y, int cx, int cy) {
//         HWND hWnd = win->m_hWnd;
//         HDC hdcScreen = ::GetDC(hWnd);
//         RECT rectDest;
//         if (WS_EX_LAYERED == (WS_EX_LAYERED & GetWindowLong(hWnd, GWL_EXSTYLE))) {
//             ::GetWindowRect(hWnd, &rectDest);
// 
//             SIZE sizeDest = { rectDest.right - rectDest.left, rectDest.bottom - rectDest.top };
//             POINT pointDest = { rectDest.left, rectDest.top };
//             POINT pointSource = { 0, 0 };
// 
//             BLENDFUNCTION blend = { 0 };
//             memset(&blend, 0, sizeof(blend));
//             blend.BlendOp = AC_SRC_OVER;
//             blend.SourceConstantAlpha = 255;
//             blend.AlphaFormat = AC_SRC_ALPHA;
//             ::UpdateLayeredWindow(hWnd, hdcScreen, &pointDest, &sizeDest, hdc, &pointSource, RGB(0, 0, 0), &blend, ULW_ALPHA);
//         }
//         else {
//             win->onPaintUpdated(hdcScreen, hdc, x, y, cx, cy);
//         }
// 
//         ::ReleaseDC(hWnd, hdcScreen);
//     }

    void onPaintUpdatedInCompositeThread(const HDC hdc, int x, int y, int cx, int cy) {
        HWND hWnd = m_hWnd;
        RECT rectDest;
        ::GetClientRect(hWnd, &rectDest);
        SIZE sizeDest = { rectDest.right - rectDest.left, rectDest.bottom - rectDest.top };
        if (0 == sizeDest.cx * sizeDest.cy)
            return;
            
        if (!m_memoryDC)
            m_memoryDC = ::CreateCompatibleDC(nullptr);

        if (!m_memoryBMP || m_clientRect.top != rectDest.top || m_clientRect.bottom != rectDest.bottom ||
            m_clientRect.right != rectDest.right || m_clientRect.left != rectDest.left) {
            m_clientRect = rectDest;

            if (m_memoryBMP)
                ::DeleteObject((HGDIOBJ)m_memoryBMP);
            m_memoryBMP = ::CreateCompatibleBitmap(hdc, sizeDest.cx, sizeDest.cy);
        }

        HBITMAP hbmpOld = (HBITMAP)::SelectObject(m_memoryDC, m_memoryBMP);
        ::BitBlt(m_memoryDC, x, y, cx, cy, hdc, x, y, SRCCOPY);
        ::SelectObject(m_memoryDC, (HGDIOBJ)hbmpOld);
    }

    void onPaintUpdatedInUiThread(int x, int y, int cx, int cy) {
        ::EnterCriticalSection(&m_memoryCanvasLock);

        HDC hdcScreen = ::GetDC(m_hWnd);
        ::BitBlt(hdcScreen, x, y, cx, cy, m_memoryDC, x, y, SRCCOPY);
        ::ReleaseDC(m_hWnd, hdcScreen);

        ::LeaveCriticalSection(&m_memoryCanvasLock);
    }

    static void staticOnPaintUpdatedInCompositeThread(wkeWebView webView, Window* win, const HDC hdc, int x, int y, int cx, int cy) {
        ::EnterCriticalSection(&win->m_memoryCanvasLock);
        win->onPaintUpdatedInCompositeThread(hdc, x, y, cx, cy);
        ::LeaveCriticalSection(&win->m_memoryCanvasLock);

        if (win->m_isLayerWindow) {
            int id = win->m_id;
            ThreadCall::callUiThreadAsync([id, win, x, y, cx, cy] {
                if (IdLiveDetect::get()->isLive(id))
                    win->onPaintUpdatedInUiThread(x, y, cx, cy);
            });
        } else {
            RECT rc = { x, y, x + cx, y + cy };
            ::InvalidateRect(win->m_hWnd, &rc, false);
        }
    }

    void onPaintMessage(HWND hWnd) {
        PAINTSTRUCT ps = { 0 };
        HDC hdc = ::BeginPaint(hWnd, &ps);

        RECT rcClip = ps.rcPaint;
        RECT rcClient;
        ::GetClientRect(hWnd, &rcClient);

        RECT rcInvalid = rcClient;
        if (rcClip.right != rcClip.left && rcClip.bottom != rcClip.top)
            ::IntersectRect(&rcInvalid, &rcClip, &rcClient);

        int srcX = rcInvalid.left - rcClient.left;
        int srcY = rcInvalid.top - rcClient.top;
        int destX = rcInvalid.left;
        int destY = rcInvalid.top;
        int width = rcInvalid.right - rcInvalid.left;
        int height = rcInvalid.bottom - rcInvalid.top;

        ::EnterCriticalSection(&m_memoryCanvasLock);
        if (0 != width && 0 != height && m_memoryBMP && m_memoryDC) {
            HBITMAP hbmpOld = (HBITMAP)::SelectObject(m_memoryDC, m_memoryBMP);
            BOOL b = ::BitBlt(hdc, destX, destY, width, height, m_memoryDC, srcX, srcY, SRCCOPY);
            b = b;
        }
        ::LeaveCriticalSection(&m_memoryCanvasLock);

        ::EndPaint(hWnd, &ps);
    }

    void onMouseMessage(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam) {
        int id = m_id;
        wkeWebView pthis = m_webContents->getWkeView();
        if (message == WM_LBUTTONDOWN || message == WM_MBUTTONDOWN || message == WM_RBUTTONDOWN) {
            ::SetFocus(hWnd);
            ::SetCapture(hWnd);
        }
        else if (message == WM_LBUTTONUP || message == WM_MBUTTONUP || message == WM_RBUTTONUP) {
            ::ReleaseCapture();
        }

        int x = LOWORD(lParam);
        int y = HIWORD(lParam);

        unsigned int flags = 0;

        if (wParam & MK_CONTROL)
            flags |= WKE_CONTROL;
        if (wParam & MK_SHIFT)
            flags |= WKE_SHIFT;

        if (wParam & MK_LBUTTON)
            flags |= WKE_LBUTTON;
        if (wParam & MK_MBUTTON)
            flags |= WKE_MBUTTON;
        if (wParam & MK_RBUTTON)
            flags |= WKE_RBUTTON;

        ThreadCall::callBlinkThreadAsync([id, pthis, message, x, y, flags] {
            if (IdLiveDetect::get()->isLive(id))
                wkeFireMouseEvent(pthis, message, x, y, flags);
        });
    }

    static LRESULT CALLBACK windowProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam) {
        Window* win = (Window *)::GetPropW(hWnd, kPrppW);
        if (!win) {
            if (message == WM_CREATE) {
                LPCREATESTRUCTW cs = (LPCREATESTRUCTW)lParam;
                Window *win = (Window *)cs->lpCreateParams;
                ThreadCall::callBlinkThreadSync([win, hWnd] {
                    wkeSetHandle(win->m_webContents->getWkeView(), hWnd);
                });
                
                ::SetPropW(hWnd, kPrppW, (HANDLE)win);
                ::SetTimer(hWnd, (UINT_PTR)win, 70, NULL);
                return 0;
            }
        }
        if (!win)
            return ::DefWindowProcW(hWnd, message, wParam, lParam);
        int id = win->m_id;

        wkeWebView pthis = win->m_webContents->getWkeView();
        if (!pthis)
            return ::DefWindowProcW(hWnd, message, wParam, lParam);
        switch (message) {
        case WM_CLOSE:
            ::ShowWindow(hWnd, SW_HIDE);
            ::DestroyWindow(hWnd);
            return 0;

        case WM_DESTROY:
            ::KillTimer(hWnd, (UINT_PTR)win);
            ::RemovePropW(hWnd, kPrppW);
            ThreadCall::callBlinkThreadSync([pthis] {
                wkeDestroyWebView(pthis);
            });
            //delete win->m_webContents;
            return 0;

        case WM_TIMER:
            //wkeRepaintIfNeeded(pthis);
            return 0;

        case WM_PAINT:
            win->onPaintMessage(hWnd);
            break;

        case WM_ERASEBKGND:
            return TRUE;

        case WM_SIZE: {
            ::EnterCriticalSection(&win->m_memoryCanvasLock);
            if (win->m_memoryDC)
                ::DeleteDC(win->m_memoryDC);
            win->m_memoryDC = nullptr;

            if (win->m_memoryBMP)
                ::DeleteObject((HGDIOBJ)win->m_memoryBMP);
            win->m_memoryBMP = nullptr;

            ::GetClientRect(hWnd, &win->m_clientRect);
            ::LeaveCriticalSection(&win->m_memoryCanvasLock);
            
            ThreadCall::callBlinkThreadSync([pthis, lParam] {
                wkeResize(pthis, LOWORD(lParam), HIWORD(lParam));
                wkeRepaintIfNeeded(pthis);
            });

            if (WindowInited == win->m_state)
                win->mate::EventEmitter<Window>::Emit("resize");
            return 0;
        }
        case WM_KEYDOWN: {
            unsigned int virtualKeyCode = wParam;
            unsigned int flags = 0;
            if (HIWORD(lParam) & KF_REPEAT)
                flags |= WKE_REPEAT;
            if (HIWORD(lParam) & KF_EXTENDED)
                flags |= WKE_EXTENDED;

            bool retVal = false;
            ThreadCall::callBlinkThreadSync([pthis, virtualKeyCode, flags, &retVal] {
                retVal = wkeFireKeyDownEvent(pthis, virtualKeyCode, flags, false);
            });
            if (retVal)
                return 0;
            break;
        }
        case WM_KEYUP: {
            unsigned int virtualKeyCode = wParam;
            unsigned int flags = 0;
            if (HIWORD(lParam) & KF_REPEAT)
                flags |= WKE_REPEAT;
            if (HIWORD(lParam) & KF_EXTENDED)
                flags |= WKE_EXTENDED;

            bool retVal = false;
            ThreadCall::callBlinkThreadSync([pthis, virtualKeyCode, flags, &retVal] {
                retVal = wkeFireKeyUpEvent(pthis, virtualKeyCode, flags, false);
            });
            if (retVal)
                return 0;
            break;
        }
        case WM_CHAR: {
            unsigned int charCode = wParam;
            unsigned int flags = 0;
            if (HIWORD(lParam) & KF_REPEAT)
                flags |= WKE_REPEAT;
            if (HIWORD(lParam) & KF_EXTENDED)
                flags |= WKE_EXTENDED;

            bool retVal = false;
            ThreadCall::callBlinkThreadSync([pthis, charCode, flags, &retVal] {
                retVal = wkeFireKeyPressEvent(pthis, charCode, flags, false);
            });
            if (retVal)
                return 0;
            break;
        }
        case WM_LBUTTONDOWN:
        case WM_MBUTTONDOWN:
        case WM_RBUTTONDOWN:
        case WM_LBUTTONDBLCLK:
        case WM_MBUTTONDBLCLK:
        case WM_RBUTTONDBLCLK:
        case WM_LBUTTONUP:
        case WM_MBUTTONUP:
        case WM_RBUTTONUP:
        case WM_MOUSEMOVE: {
            win->onMouseMessage(hWnd, message, wParam, lParam);
            break;
        }
        case WM_CONTEXTMENU: {
            POINT pt;
            pt.x = LOWORD(lParam);
            pt.y = HIWORD(lParam);

            if (pt.x != -1 && pt.y != -1)
                ::ScreenToClient(hWnd, &pt);

            unsigned int flags = 0;

            if (wParam & MK_CONTROL)
                flags |= WKE_CONTROL;
            if (wParam & MK_SHIFT)
                flags |= WKE_SHIFT;

            if (wParam & MK_LBUTTON)
                flags |= WKE_LBUTTON;
            if (wParam & MK_MBUTTON)
                flags |= WKE_MBUTTON;
            if (wParam & MK_RBUTTON)
                flags |= WKE_RBUTTON;

            ThreadCall::callBlinkThreadAsync([id, pthis, pt, flags] {
                if (IdLiveDetect::get()->isLive(id))
                    wkeFireContextMenuEvent(pthis, pt.x, pt.y, flags);
            });
            break;
        }
        case WM_MOUSEWHEEL: {
            POINT pt;
            pt.x = LOWORD(lParam);
            pt.y = HIWORD(lParam);
            ::ScreenToClient(hWnd, &pt);

            int delta = GET_WHEEL_DELTA_WPARAM(wParam);

            unsigned int flags = 0;

            if (wParam & MK_CONTROL)
                flags |= WKE_CONTROL;
            if (wParam & MK_SHIFT)
                flags |= WKE_SHIFT;

            if (wParam & MK_LBUTTON)
                flags |= WKE_LBUTTON;
            if (wParam & MK_MBUTTON)
                flags |= WKE_MBUTTON;
            if (wParam & MK_RBUTTON)
                flags |= WKE_RBUTTON;

            ThreadCall::callBlinkThreadAsync([id, pthis, pt, delta, flags] {
                wkeFireMouseWheelEvent(pthis, pt.x, pt.y, delta, flags);
            });
            break;
        }
        case WM_SETFOCUS:
            ThreadCall::callBlinkThreadAsync([id, pthis]{
                if (IdLiveDetect::get()->isLive(id))
                    wkeSetFocus(pthis);
            });
            return 0;

        case WM_KILLFOCUS:
            ThreadCall::callBlinkThreadAsync([id, pthis] {
                if (IdLiveDetect::get()->isLive(id))
                    wkeKillFocus(pthis);
            });
            return 0;

        case WM_SETCURSOR: {
            bool retVal = false;
            ThreadCall::callBlinkThreadSync([pthis, hWnd, &retVal] {
                retVal = wkeFireWindowsMessage(pthis, hWnd, WM_SETCURSOR, 0, 0, nullptr);
            });
            if (retVal)
                return 0;
        }
            break;

        case WM_IME_STARTCOMPOSITION: {
            wkeRect caret;
            ThreadCall::callBlinkThreadSync([pthis, &caret] {
                caret = wkeGetCaretRect(pthis);
            });

            COMPOSITIONFORM compositionForm;
            compositionForm.dwStyle = CFS_POINT | CFS_FORCE_POSITION;
            compositionForm.ptCurrentPos.x = caret.x;
            compositionForm.ptCurrentPos.y = caret.y;

            HIMC hIMC = ::ImmGetContext(hWnd);
            ::ImmSetCompositionWindow(hIMC, &compositionForm);
            ::ImmReleaseContext(hWnd, hIMC);
        }
            return 0;
        }

        return ::DefWindowProcW(hWnd, message, wParam, lParam);
    }

    struct CreateWindowParam {
        int x;
        int y;
        int width;
        int height;
        unsigned styles;
        unsigned styleEx;
        bool transparent;
        std::wstring title;
    };

    static void staticDidCreateScriptContextCallback(wkeWebView webView, void* param, void* frame, void* context, int extensionGroup, int worldId) {
        Window* self = (Window*)param;
        self->onDidCreateScriptContext(webView, frame, (v8::Local<v8::Context>*)context, extensionGroup, worldId);
    }
    
    void onDidCreateScriptContext(wkeWebView webView, void* frame, v8::Local<v8::Context>* context, int extensionGroup, int worldId) {
        if (m_nodeBinding)
            return;

        BlinkMicrotaskSuppressionHandle handle = nodeBlinkMicrotaskSuppressionEnter((*context)->GetIsolate());
        m_nodeBinding = new NodeBindings(false, ThreadCall::blinkLoop());
        node::Environment* env = m_nodeBinding->createEnvironment(*context);
        node::LoadEnvironment(env);
        nodeBlinkMicrotaskSuppressionLeave(handle);
    }

    static v8::Local<v8::Value> toBuffer(v8::Isolate* isolate, void* val, int size) {
        auto buffer = node::Buffer::Copy(isolate, static_cast<char*>(val), size);
        if (buffer.IsEmpty()) {
            return v8::Null(isolate);
        } else {
            return buffer.ToLocalChecked();
        }
    }

    static Window* newWindow(gin::Dictionary* options, v8::Local<v8::Object> wrapper) {
        Window* win = new Window(options->isolate(), wrapper);
        CreateWindowParam createWindowParam;
        createWindowParam.styles = 0;
        createWindowParam.styleEx = 0;
        createWindowParam.transparent = false;

        WebContents* webContents = nullptr;
        Handle<Object> webContentsV8;
        // If no WebContents was passed to the constructor, create it from options.
        if (!options->Get("webContents", &webContentsV8)) {
            // Use options.webPreferences to create WebContents.
            gin::Dictionary webPreferences = gin::Dictionary::CreateEmpty(options->isolate());
            options->Get(options::kWebPreferences, &webPreferences);

            // Copy the backgroundColor to webContents.
            v8::Local<v8::Value> value;
            if (options->Get(options::kBackgroundColor, &value))
                webPreferences.Set(options::kBackgroundColor, value);

            v8::Local<v8::Value> transparent;
            if (options->Get("transparent", &transparent))
                webPreferences.Set("transparent", transparent);

            // Offscreen windows are always created frameless.
            bool offscreen;
            if (webPreferences.Get("offscreen", &offscreen) && offscreen) {
                options->Set(options::kFrame, false);
            }
            webContents = WebContents::create(options->isolate(), webPreferences);
        } else
            DebugBreak();
            //webContents = WebContents::ObjectWrap::Unwrap<WebContents>(webContentsV8);

        win->m_webContents = webContents;

        v8::Local<v8::Value> transparent;
        options->Get("transparent", &transparent);
        v8::Local<v8::Value> height;
        options->Get("height", &height);
        v8::Local<v8::Value> width;
        options->Get("width", &width);
        v8::Local<v8::Value> x;
        options->Get("x", &x);
        v8::Local<v8::Value> y;
        options->Get("y", &y);
        v8::Local<v8::Value> title;
        options->Get("title", &title);
        if (title->IsString()) {
            v8::String::Utf8Value str(title);
            createWindowParam.title = StringUtil::UTF8ToUTF16(*str);
        } else 
            createWindowParam.title = L"Electron";
        
        createWindowParam.x = x->Int32Value();
        createWindowParam.y = y->Int32Value();
        createWindowParam.width = width->Int32Value();
        createWindowParam.height = height->Int32Value();

        if (transparent->IsBoolean() && transparent->ToBoolean()->BooleanValue()) {
            createWindowParam.transparent = true;
            createWindowParam.styles = WS_POPUP;
            createWindowParam.styleEx = WS_EX_LAYERED;
        } else {
            createWindowParam.styles = WS_OVERLAPPEDWINDOW;
            createWindowParam.styleEx = 0;
        }

        //ThreadCall::callUiThreadSync([win, &createWindowParam] {
            win->newWindowTaskInUiThread(&createWindowParam);
        //});
        return win;
    }

    void newWindowTaskInUiThread(const CreateWindowParam* createWindowParam) {
        m_hWnd = ::CreateWindowEx(
            createWindowParam->styleEx,        // window ex-style
            L"mb_electron_window",    // window class name
            createWindowParam->title.c_str(), // window caption
            createWindowParam->styles,         // window style
            createWindowParam->x,              // initial x position
            createWindowParam->y,              // initial y position
            createWindowParam->width,          // initial x size
            createWindowParam->height,         // initial y size
            NULL,         // parent window handle
            NULL,           // window menu handle
            ::GetModuleHandleW(NULL),           // program instance handle
            this);         // creation parameters

        if (!::IsWindow(m_hWnd))
            return;

        m_clientRect.right = createWindowParam->width;
        m_clientRect.bottom = createWindowParam->height;

        Window* win = this;
        ThreadCall::callBlinkThreadSync([win, createWindowParam] {
            if (createWindowParam->transparent)
                wkeSetTransparent(win->m_webContents->getWkeView(), true);
            wkeSettings settings;
            settings.mask = WKE_SETTING_PAINTCALLBACK_IN_OTHER_THREAD;
            wkeConfigure(&settings);
            wkeResize(win->m_webContents->getWkeView(), createWindowParam->width, createWindowParam->height);
            wkeOnPaintUpdated(win->m_webContents->getWkeView(), (wkePaintUpdatedCallback)staticOnPaintUpdatedInCompositeThread, win);
            wkeOnDidCreateScriptContext(win->m_webContents->getWkeView(), staticDidCreateScriptContextCallback, win);
        });

        ::ShowWindow(m_hWnd, TRUE);
        m_state = WindowInited;
    }

private:
    static void newFunction(const v8::FunctionCallbackInfo<v8::Value>& args) {
        Isolate* isolate = args.GetIsolate();
        HandleScope scope(isolate);

        if (args.IsConstructCall()) {
            if (args.Length() > 1)
                return;
            
            gin::Dictionary options(isolate, args[0]->ToObject()); // 使用new调用 `new Point(...)`
            Window* win = newWindow(&options, args.This());
            WindowList::GetInstance()->AddWindow(win);
            
			// win->Wrap(args.This(), isolate); // 包装this指针 // weolar
            args.GetReturnValue().Set(args.This());
            // args.GetReturnValue().Set(win->GetWrapper());
        } else {
            // 使用`Point(...)`
            const int argc = 2;
            Local<Value> argv[argc] = { args[0], args[1] };
            // 使用constructor构建Function
            Local<Function> cons = Local<Function>::New(isolate, constructor);
            args.GetReturnValue().Set(cons->NewInstance(argc, argv));
            DebugBreak();
        }
    }

    void closeApi() {
        ::SendMessage(m_hWnd, WM_CLOSE, 0, 0);
    }

    void focusApi() {
        ::SetFocus(m_hWnd);
    }

    void blurApi() {
        ::SetFocus(NULL);
    }

    bool isFocusedApi() {
        return ::GetFocus() == m_hWnd;
    }

    void showApi() {
        ::ShowWindow(m_hWnd, TRUE);
        ::SetFocus(m_hWnd);
    }

    void showInactiveApi() {
        ::ShowWindow(m_hWnd, TRUE);
    }

    void hideApi() {
        ::ShowWindow(m_hWnd, FALSE);
    }

    bool isVisibleApi() {
        return !!::IsWindowVisible(m_hWnd);
    }

    //isEnabled
    bool isEnabledApi() {
        return !!::IsWindowEnabled(m_hWnd);
    }

    //maximize
    void maximizeApi() {
        ::ShowWindow(m_hWnd, SW_MAXIMIZE);
    }

    void unmaximizeApi() {
        ::ShowWindow(m_hWnd, SW_RESTORE);
    }

    bool isMaximizedApi() {
        return !!::IsZoomed(m_hWnd);
    }

    void minimizeApi() {
        ::ShowWindow(m_hWnd, SW_MINIMIZE);
    }

    //restore
    void restoreApi() {
        ::ShowWindow(m_hWnd, SW_RESTORE);
    }

    //isMinimized
    bool isMinimizedApi() {
        return !!IsIconic(m_hWnd);
    }

    //setFullScreen
    void setFullScreenApi(bool b) {
        if (b) {
            RECT rc;
            HWND hDesk = ::GetDesktopWindow();
            ::GetWindowRect(hDesk, &rc);
            ::SetWindowLong(m_hWnd, GWL_STYLE, GetWindowLong(m_hWnd, GWL_STYLE) | WS_BORDER);
            ::SetWindowPos(m_hWnd, HWND_TOPMOST, 0, 0, rc.right, rc.bottom, SWP_SHOWWINDOW);
        } else {
            ::SetWindowLong(m_hWnd, GWL_STYLE, GetWindowLong(m_hWnd, GWL_STYLE) ^ WS_BORDER);
        }
    }

    static void isFullScreenApi() {
        ::DebugBreak();
    }

    void setParentWindowApi() {
    }

    void getParentWindowApi() {
    }

    void getChildWindowsApi() {
    }

    bool isModalApi() {
        return false;
    }

    UINT_PTR getNativeWindowHandleApi() {
        //args.GetReturnValue().Set(toBuffer(isolate, (void*)(&win->m_hWnd), sizeof(HWND)));
        return (UINT_PTR)m_hWnd;
    }

    Local<Object> getBoundsApi() {
        RECT clientRect;
        ::EnterCriticalSection(&m_memoryCanvasLock);
        clientRect = m_clientRect;
        ::LeaveCriticalSection(&m_memoryCanvasLock);
        Local<Integer> x = Integer::New(isolate(), clientRect.left);
        Local<Integer> y = Integer::New(isolate(), clientRect.top);
        Local<Integer> width = Integer::New(isolate(), clientRect.right - clientRect.left);
        Local<Integer> height = Integer::New(isolate(), clientRect.bottom - clientRect.top);
        Local<Object> bounds = Object::New(isolate());
        bounds->Set(String::NewFromUtf8(isolate(), "x"), x);
        bounds->Set(String::NewFromUtf8(isolate(), "y"), y);
        bounds->Set(String::NewFromUtf8(isolate(), "width"), width);
        bounds->Set(String::NewFromUtf8(isolate(), "height"), height);
        return bounds;
    }

    void setBoundsApi(v8::Local<v8::Object> bounds) {
        LONG x = (LONG)bounds->Get(String::NewFromUtf8(isolate(), "x"))->NumberValue();
        LONG y = (LONG)bounds->Get(String::NewFromUtf8(isolate(), "y"))->NumberValue();
        LONG width = (LONG)bounds->Get(String::NewFromUtf8(isolate(), "width"))->NumberValue();
        LONG height = (LONG)bounds->Get(String::NewFromUtf8(isolate(), "height"))->NumberValue();
        ::MoveWindow(m_hWnd, x, y, width, height, TRUE);
    }

    v8::Local<v8::Object> getSizeApi() {
        RECT clientRect;
        ::EnterCriticalSection(&m_memoryCanvasLock);
        clientRect = m_clientRect;
        ::LeaveCriticalSection(&m_memoryCanvasLock);
        v8::Local<v8::Integer> width = Integer::New(isolate(), clientRect.right - clientRect.left);
        v8::Local<v8::Integer> height = Integer::New(isolate(), clientRect.bottom - clientRect.top);
        v8::Local<v8::Array> size = Array::New(isolate(), 2);
        size->Set(0, width);
        size->Set(1, height);
        return size;
    }

    void setSizeApi(int32_t width, int32_t height) {
        ::SetWindowPos(m_hWnd, NULL, 0, 0, width, height, SWP_NOMOVE);
    }

    void getContentBoundsApi() {
    }

    void setContentBoundsApi() {
    }

    std::vector<int> getContentSizeApi() {
        Window* win = this;
        int width;
        int height;
        ThreadCall::callBlinkThreadSync([win, &width, &height] {
            width = wkeGetContentWidth(win->m_webContents->getWkeView());
            height = wkeGetContentHeight(win->m_webContents->getWkeView());
        });
        std::vector<int> size = { width, height };
        return size;
    }

    void setContentSizeApi(int width, int height) {
        Window* win = this;
        int id = win->m_id;
        ThreadCall::callBlinkThreadAsync([win, id, width, height] {
            if (!IdLiveDetect::get()->isLive(id))
                return;
            wkeResize(win->m_webContents->getWkeView(), width, height);
            wkeRepaintIfNeeded(win->m_webContents->getWkeView());
        });
    }

    void setMinimumSizeApi() {
    }

    void getMinimumSizeApi() {
    }

    void setMaximumSizeApi() {
    }

    void getMaximumSizeApi() {
    }

    void setResizableApi() {
    }

    void isResizableApi() {
    }

    void setMovableApi() {
    }

    void isMovableApi() {
    }

    void setMinimizableApi() {
    }

    void isMinimizableApi() {
    }

    void isMaximizableApi() {
    }

    void setFullScreenableApi() {
    }

    void isFullScreenableApi() {
    }

    void setClosableApi() {
    }

    void isClosableApi() {
    }

    void setAlwaysOnTopApi(bool b) {
        ::SetWindowPos(m_hWnd, b ? HWND_TOPMOST : HWND_NOTOPMOST, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE);
    }

    bool isAlwaysOnTopApi() {
        return 0 == (::GetWindowLong(m_hWnd, GWL_EXSTYLE) & WS_EX_TOPMOST);
    }

    void centerApi() {
        int screenX, screenY;
        screenX = ::GetSystemMetrics(SM_CXSCREEN);  //取得屏幕的宽度
        screenY = ::GetSystemMetrics(SM_CYSCREEN);  //取得屏幕的高度

        RECT rect;
        ::GetWindowRect(m_hWnd, &rect);
        rect.left = (screenX - rect.right) / 2;
        rect.top = (screenY - rect.bottom) / 2;

        //设置窗体位置
        ::SetWindowPos(m_hWnd, NULL, rect.left, rect.top, rect.right, rect.bottom, SWP_NOSIZE);
    }

    void setPositionApi(int x, int y) {
        ::SetWindowPos(m_hWnd, NULL, x, y, 0, 0, SWP_NOSIZE);
    }

    std::vector<int> getPositionApi() {
        RECT rect = {0};
        ::GetWindowRect(m_hWnd, &rect);
        std::vector<int> pos = { rect.left, rect.top };
        return pos;
    }

    void setTitleApi(const std::string& title) {
        std::wstring titleW;
        titleW = StringUtil::UTF8ToUTF16(title);
        ::SetWindowText(m_hWnd, titleW.c_str());
    }

    std::string getTitleApi() {
        std::vector<wchar_t> titleW;
        titleW.resize(MAX_PATH + 1);
        ::GetWindowText(m_hWnd, &titleW[0], MAX_PATH);
        std::string titleA;
        titleA = StringUtil::UTF16ToUTF8(std::wstring(&titleW[0], titleW.size()));
        return titleA;
    }

    void flashFrameApi() {
    }

    void setSkipTaskbarApi(bool b) {
        DWORD style = ::GetWindowLong(m_hWnd, GWL_STYLE);
        if (b) {
            style |= WS_EX_TOOLWINDOW;
            style &= ~WS_EX_APPWINDOW;
        } else {  //todo 如果窗口原来的style没有WS_EX_APPWINDOW，就可能有问题
            style &= ~WS_EX_TOOLWINDOW;
            style |= WS_EX_APPWINDOW;
        }
        ::SetWindowLong(m_hWnd, GWL_EXSTYLE, style);
    }

    void setBackgroundColorApi() {
    }

    void setDocumentEditedApi(bool b) {
        Window* win = this;
        ThreadCall::callBlinkThreadSync([win, b] {
            wkeSetEditable(win->m_webContents->getWkeView(), b);
        });
    }

    void isDocumentEditedApi() {
    }

    void setIgnoreMouseEventsApi() {
    }

    void setContentProtectionApi() {
    }

    void setFocusableApi() {
    }

    void focusOnWebViewApi() {
        Window* win = this;
        ThreadCall::callBlinkThreadSync([win] {
            wkeSetFocus(win->m_webContents->getWkeView());
        });
    }

    void isWebViewFocusedApi() {
    }

    void setOverlayIconApi() {
    }

    void setThumbarButtonsApi() {
    }

    void setMenuApi() {
    }

    void setAutoHideMenuBarApi() {
    }

    void isMenuBarAutoHideApi() {
    }

    void setMenuBarVisibilityApi() {
    }

    void isMenuBarVisibleApi() {
    }

    void setVisibleOnAllWorkspacesApi() {
    }

    void isVisibleOnAllWorkspacesApi() {
    }

    void hookWindowMessageApi() {
    }

    void isWindowMessageHookedApi() {
    }

    void unhookWindowMessageApi() {
    }

    void unhookAllWindowMessagesApi() {
    }

    void setThumbnailClipApi() {
    }

    void setThumbnailToolTipApi() {
    }

    void setAppDetailsApi() {
    }

    void setIconApi() {
    }

    v8::Local<v8::Value> getWebContentsApi() {
        if (!m_webContents)
            return v8::Null(isolate());
        else
            return v8::Local<v8::Value>::New(isolate(), m_webContents->GetWrapper());
    }

    // 空实现
    void nullFunction() {
    }

    static v8::Persistent<v8::Function> constructor;

public:
    static gin::WrapperInfo kWrapperInfo;

private:
    enum WindowState {
        WindowUninited,
        WindowInited,
        WindowDestroying,
        WindowDestroyed
    };
    WindowState m_state;
    static const WCHAR* kPrppW;
    WebContents* m_webContents;
    NodeBindings* m_nodeBinding;
    
    HWND m_hWnd;
    CRITICAL_SECTION m_memoryCanvasLock;
    HBITMAP m_memoryBMP;
    HDC m_memoryDC;
    RECT m_clientRect;
    bool m_isLayerWindow;
    int m_id;
};

const WCHAR* Window::kPrppW = L"mele";
Persistent<Function> Window::constructor;
gin::WrapperInfo Window::kWrapperInfo = { gin::kEmbedderNativeGin };

static void initializeWindowApi(Local<Object> target, Local<Value> unused, Local<Context> context, const NodeNative* native) {
    Environment* env = Environment::GetCurrent(context);
    Window::init(target, env);
    WNDCLASS wndClass = { 0 };
    if (!GetClassInfoW(NULL, L"mb_electron_window", &wndClass)) {
        wndClass.style = CS_HREDRAW | CS_VREDRAW;
        wndClass.lpfnWndProc = &Window::windowProc;
        wndClass.cbClsExtra = 200;
        wndClass.cbWndExtra = 200;
        wndClass.hInstance = GetModuleHandleW(NULL);
        //wndClass.hIcon = LoadIcon(GetModuleHandleW(NULL), MAKEINTRESOURCE(IDI_ICON1));
        wndClass.hCursor = LoadCursor(NULL, IDC_ARROW);
        wndClass.hbrBackground = NULL;
        wndClass.lpszMenuName = NULL;
        wndClass.lpszClassName = L"mb_electron_window";
        RegisterClass(&wndClass);
    }
}

#pragma warning(push)
#pragma warning(disable:4309)
#pragma warning(disable:4838)
static const char BrowserWindowNative[] =
"//const {EventEmitter} = require('events');"
"//const {BrowserWindow} = process.binding('atom_browser_window');"
"//Object.setPrototypeOf(BrowserWindow.prototype, EventEmitter.prototype);"
"//module.exports = BrowserWindow;";
#pragma warning(pop)

static NodeNative nativeBrowserWindowNative{ "BrowserWindow", BrowserWindowNative, sizeof(BrowserWindowNative) - 1 };

NODE_MODULE_CONTEXT_AWARE_BUILTIN_SCRIPT_MANUAL(atom_browser_window, initializeWindowApi, &nativeBrowserWindowNative)

} // atom