/////////////////////////////////////////////////////////////////////////////
// Name:        src/common/mstream.cpp
// Purpose:     "Memory stream" classes
// Author:      Guilhem Lavaux
// Modified by: VZ (23.11.00): general code review
// Created:     04/01/98
// Copyright:   (c) Guilhem Lavaux
// Licence:     wxWindows licence
/////////////////////////////////////////////////////////////////////////////

// ============================================================================
// declarations
// ============================================================================

// ----------------------------------------------------------------------------
// headers
// ----------------------------------------------------------------------------

// For compilers that support precompilation, includes "wx.h".
#include "wx/wxprec.h"


#if wxUSE_STREAMS

#include "wx/mstream.h"

#ifndef   WX_PRECOMP
    #include  "wx/stream.h"
#endif  //WX_PRECOMP

#include <stdlib.h>

// ============================================================================
// implementation
// ============================================================================

// ----------------------------------------------------------------------------
// wxMemoryInputStream
// ----------------------------------------------------------------------------

wxIMPLEMENT_ABSTRACT_CLASS(wxMemoryInputStream, wxInputStream);

wxMemoryInputStream::wxMemoryInputStream(const void *data, size_t len)
{
    m_i_streambuf = new wxStreamBuffer(wxStreamBuffer::read);
    m_i_streambuf->SetBufferIO(const_cast<void *>(data), len);
    m_i_streambuf->SetIntPosition(0); // seek to start pos
    m_i_streambuf->Fixed(true);

    m_length = len;
}

wxMemoryInputStream::wxMemoryInputStream(const wxMemoryOutputStream& stream)
{
    const wxFileOffset lenFile = stream.GetLength();
    if ( lenFile == wxInvalidOffset )
    {
        m_i_streambuf = NULL;
        m_lasterror = wxSTREAM_EOF;
        return;
    }

    const size_t len = wx_truncate_cast(size_t, lenFile);
    wxASSERT_MSG( len == lenFile + size_t(0), wxT("huge files not supported") );

    m_i_streambuf = new wxStreamBuffer(wxStreamBuffer::read);
    m_i_streambuf->SetBufferIO(len); // create buffer
    stream.CopyTo(m_i_streambuf->GetBufferStart(), len);
    m_i_streambuf->SetIntPosition(0); // seek to start pos
    m_i_streambuf->Fixed(true);
    m_length = len;
}

void
wxMemoryInputStream::InitFromStream(wxInputStream& stream, wxFileOffset lenFile)
{
    if ( lenFile == wxInvalidOffset )
        lenFile = stream.GetLength();

    if ( lenFile == wxInvalidOffset )
    {
        m_i_streambuf = NULL;
        m_lasterror = wxSTREAM_EOF;
        return;
    }

    const size_t len = wx_truncate_cast(size_t, lenFile);
    wxASSERT_MSG( (wxFileOffset)len == lenFile, wxT("huge files not supported") );

    m_i_streambuf = new wxStreamBuffer(wxStreamBuffer::read);
    m_i_streambuf->SetBufferIO(len); // create buffer
    stream.Read(m_i_streambuf->GetBufferStart(), len);
    m_i_streambuf->SetIntPosition(0); // seek to start pos
    m_i_streambuf->Fixed(true);
    m_length = stream.LastRead();
}

bool wxMemoryInputStream::CanRead() const
{
    return m_i_streambuf->GetIntPosition() != m_length;
}

wxMemoryInputStream::~wxMemoryInputStream()
{
    delete m_i_streambuf;
}

char wxMemoryInputStream::Peek()
{
    char *buf = (char *)m_i_streambuf->GetBufferStart();
    size_t pos = m_i_streambuf->GetIntPosition();
    if ( pos == m_length )
    {
        m_lasterror = wxSTREAM_READ_ERROR;

        return 0;
    }

    return buf[pos];
}

size_t wxMemoryInputStream::OnSysRead(void *buffer, size_t nbytes)
{
    size_t pos = m_i_streambuf->GetIntPosition();
    if ( pos == m_length )
    {
        m_lasterror = wxSTREAM_EOF;

        return 0;
    }

    m_i_streambuf->Read(buffer, nbytes);
    m_lasterror = wxSTREAM_NO_ERROR;

    return m_i_streambuf->GetIntPosition() - pos;
}

wxFileOffset wxMemoryInputStream::OnSysSeek(wxFileOffset pos, wxSeekMode mode)
{
    return m_i_streambuf->Seek(pos, mode);
}

wxFileOffset wxMemoryInputStream::OnSysTell() const
{
    return m_i_streambuf->Tell();
}

// ----------------------------------------------------------------------------
// wxMemoryOutputStream
// ----------------------------------------------------------------------------

wxIMPLEMENT_DYNAMIC_CLASS(wxMemoryOutputStream, wxOutputStream);

wxMemoryOutputStream::wxMemoryOutputStream(void *data, size_t len)
{
    m_o_streambuf = new wxStreamBuffer(wxStreamBuffer::write);
    if ( data )
        m_o_streambuf->SetBufferIO(data, len);
    m_o_streambuf->Fixed(false);
    m_o_streambuf->Flushable(false);
}

wxMemoryOutputStream::~wxMemoryOutputStream()
{
    delete m_o_streambuf;
}

size_t wxMemoryOutputStream::OnSysWrite(const void *buffer, size_t nbytes)
{
    size_t oldpos = m_o_streambuf->GetIntPosition();
    m_o_streambuf->Write(buffer, nbytes);
    size_t newpos = m_o_streambuf->GetIntPosition();

    // FIXME can someone please explain what this does? (VZ)
    if ( !newpos )
        newpos = m_o_streambuf->GetBufferSize();

    return newpos - oldpos;
}

wxFileOffset wxMemoryOutputStream::OnSysSeek(wxFileOffset pos, wxSeekMode mode)
{
    return m_o_streambuf->Seek(pos, mode);
}

wxFileOffset wxMemoryOutputStream::OnSysTell() const
{
    return m_o_streambuf->Tell();
}

size_t wxMemoryOutputStream::CopyTo(void *buffer, size_t len) const
{
    wxCHECK_MSG( buffer, 0, wxT("must have buffer to CopyTo") );

    if ( len > GetSize() )
        len = GetSize();

    memcpy(buffer, m_o_streambuf->GetBufferStart(), len);

    return len;
}

#endif // wxUSE_STREAMS
