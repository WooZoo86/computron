// Computron x86 PC Emulator
// Copyright (C) 2003-2018 Andreas Kling <awesomekling@gmail.com>
//
// Redistribution and use in source and binary forms, with or without
// modification, are permitted provided that the following conditions
// are met:
// 1. Redistributions of source code must retain the above copyright
//    notice, this list of conditions and the following disclaimer.
// 2. Redistributions in binary form must reproduce the above copyright
//    notice, this list of conditions and the following disclaimer in the
//    documentation and/or other materials provided with the distribution.
//
// THIS SOFTWARE IS PROVIDED BY ANDREAS KLING ``AS IS'' AND ANY
// EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
// IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
// PURPOSE ARE DISCLAIMED.  IN NO EVENT SHALL ANDREAS KLING OR
// CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
// EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
// PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
// PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY
// OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
// (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
// OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

#pragma once

#include "iodevice.h"

class CPU;

class PIC final : public IODevice
{
public:
    PIC(bool isMaster, Machine&);
    ~PIC();

    virtual void reset() override;
    void out8(WORD port, BYTE data) override;
    BYTE in8(WORD port) override;

    void raise(BYTE num);
    void lower(BYTE num);

    BYTE getIMR() const { return m_imr; }
    BYTE getIRR() const { return m_irr; }
    BYTE getISR() const { return m_isr; }

    void dumpMask();
    void unmaskAll();

    static void serviceIRQ(CPU&);
    static void raiseIRQ(Machine&, BYTE num);
    static void lowerIRQ(Machine&, BYTE num);
    static bool isIRQRaised(Machine&, BYTE num);
    static bool isIgnoringAllIRQs();
    static void setIgnoreAllIRQs(bool);
    static bool hasPendingIRQ() { return s_pendingRequests; }

    PIC& master() const;
    PIC& slave() const;

private:
    static void updatePendingRequests(Machine&);

    void writePort0(BYTE);
    void writePort1(BYTE);

    WORD m_baseAddress { 0 };
    BYTE m_isrBase { 0 };
    BYTE m_irqBase { 0 };

    BYTE m_isr { 0 };
    BYTE m_irr { 0 };
    BYTE m_imr { 0 };

    bool m_icw2Expected { false };
    bool m_icw4Expected { false };
    bool m_readISR { false };
    bool m_specialMaskMode { false };
    bool m_isMaster { false };

    static std::atomic<WORD> s_pendingRequests;
};
