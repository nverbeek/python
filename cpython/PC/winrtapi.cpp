#include "Python.h"

#ifdef MS_WINRT

#include <Windows.h>

using namespace Platform;
using namespace Windows::ApplicationModel;
using namespace Windows::System::UserProfile;
using namespace Windows::Storage;
using namespace Windows::Storage::Streams;
using namespace Windows::Security::Cryptography;

extern "C" {
    
    void win32_urandom(unsigned char *buffer, Py_ssize_t size, int raise)
    {
        IBuffer^ data = CryptographicBuffer::GenerateRandom(size);
        Array<unsigned char>^ data2;
        CryptographicBuffer::CopyToByteArray(data, &data2);
        for(int i=0; i < size; i++)
            buffer[i] = data2[i];
    }

    /*BOOL*/ int winrt_startfile(const wchar_t *operation, const wchar_t *path)
    {
        /* TODO: Implement launcher */
        return FALSE;
    }

    size_t winrt_getinstallpath(wchar_t *buffer, size_t cch)
    {
        try
        {
            String^ path = Package::Current->InstalledLocation->Path;
            wcscpy_s(buffer, cch, path->Data());
            return path->Length();
        }
        catch (Exception^)
        {
            return 0;
        }
    }

    static bool set_item(PyObject *d, const wchar_t *name, String^ (*value_func)())
    {
        PyObject *valueobj;
        try
        {
            auto value = value_func();
            valueobj = PyUnicode_FromWideChar(value->Data(), -1);
        }
        catch (Exception^)
        {
            valueobj = PyUnicode_FromString("");
        }
    
        if (!valueobj)
            return false;

        auto nameobj = PyUnicode_FromWideChar(name, -1);
        if (!nameobj)
        {
            Py_DECREF(valueobj);
            return false;
        }

        bool success = PyDict_SetItem(d, nameobj, valueobj) == 0;
        Py_DECREF(nameobj);
        Py_DECREF(valueobj);
        return success;
    }

    PyObject * winrt_defaultenviron()
    {
        auto d = PyDict_New();

        if (d != nullptr &&
            set_item(d, L"INSTALLPATH", [] { return Package::Current->InstalledLocation->Path; }) &&
            set_item(d, L"APPDATA", [] { return ApplicationData::Current->RoamingFolder->Path; }) &&
            set_item(d, L"LOCALAPPDATA", [] { return ApplicationData::Current->LocalFolder->Path; }) &&
            set_item(d, L"TEMP", [] { return ApplicationData::Current->TemporaryFolder->Path; }) &&
            set_item(d, L"TMP", [] { return ApplicationData::Current->TemporaryFolder->Path; })
            )
            return d;

        Py_DECREF(d);
        return nullptr;
    }
}

#endif
