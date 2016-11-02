//  File:   GPIO.h
//  Abstracts access to a digital GPIO pin.
//
//  This file is part of the Ember firmware.
//
//  Copyright 2016 Autodesk, Inc. <http://ember.autodesk.com/>
//    
//  Authors:
//  Jason Lefley
//
//  This program is free software; you can redistribute it and/or
//  modify it under the terms of the GNU General Public License
//  as published by the Free Software Foundation; either version 2
//  of the License, or (at your option) any later version.
//
//  THIS PROGRAM IS DISTRIBUTED IN THE HOPE THAT IT WILL BE USEFUL,
//  BUT WITHOUT ANY WARRANTY; WITHOUT EVEN THE IMPLIED WARRANTY OF
//  MERCHANTABILITY OR FITNESS FOR A PARTICULAR PURPOSE.  SEE THE
//  GNU GENERAL PUBLIC LICENSE FOR MORE DETAILS.
//
//  You should have received a copy of the GNU General Public License
//  along with this program; if not, see <http://www.gnu.org/licenses/>.

#ifndef GPIO_H
#define GPIO_H

class GPIO {
public:
    GPIO(int gpioNumber);
    ~GPIO();
    void SetDirectionOut();
    void SetOutputHigh();

private:
    GPIO(const GPIO&);
    GPIO& operator=(const GPIO&);

    int _gpioNumber;
};

#endif /* GPIO_H */

