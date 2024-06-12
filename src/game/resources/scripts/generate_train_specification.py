from cpp_writer import CPPWriter

import struct

data = b'\x08\x07\xd0\x07\x05\x00\x00\x00\x00\x00\x08\x07\x3a\x07\x03\x07\x0a\x0c\x07\x07\x1c\x07\x9e\x07\x04\x08\x0a\x0b\x0c\x0d\x53\x07\x94\x07\x05\x07\x07\x07\x08\x08\x89\x07\xb2\x07\x06\x08\x0a\x0b\x0c\x0d\x80\x07\xc9\x07\x06\x08\x0a\x0b\x0c\x0d\xb7\x07\xd0\x07\x07\x09\x09\x09\x09\x09\x08\x07\x8f\x07\x00\x01\x02\x01\x02\x03\x3a\x07\xc9\x07\x00\x02\x05\x04\x05\x04\xb7\x07\xd0\x07\x00\x06\x06\x06\x06\x06\x08\x07\x9e\x07\x00\x02\x05\x04\x05\x04\x26\x07\xc9\x07\x00\x02\x05\x04\x05\x04\x08\x07\x9e\x07\x00\x02\x05\x04\x05\x04\x26\x07\xc9\x07\x00\x02\x05\x04\x05\x04'

types = {
    0: 'Server',
    1: 'AncientLocomotive',
    2: 'SteamLocomotive',
    3: 'Trolley',
    4: 'DieselLocomotive',
    5: 'ElectricLocomotive',
    6: 'HighSpeedLocomotive',

    7: 'AncientPassengerCarriage',
    8: 'PassengerCarriage',
    9: 'HighSpeedPassengerCarriage',
    10: 'OpenFreightCarriage',
    11: 'CoveredFreightCarriage',
    12: 'PocketWagon',
    13: 'TankWagon',

    14: 'CrashedTrain'
}

with CPPWriter('train_specification.cpp') as w:
    w.writeln('#include "train_specification.h"')
    w.writeln()
    w.writeln('#include <game/train.h>')
    w.writeln()
    w.writeln('namespace resl {')
    w.writeln()
    w.writeln('/* 1d32:0000 : 140 bytes */')
    w.writeln('const TrainSpecification g_trainSpecifications[14] = {')
    with w.indent():
        for i in range(14):
            vals = struct.unpack('<hhBBBBBB', data[:10])
            minYear, maxYear, maxSpeed = vals[:3]
            w.writeln('{ // %s' % types[i])
            with w.indent():
                w.writeln('%d, %d, %d,' % (minYear, maxYear, maxSpeed))
                w.writeln('{')
                with w.indent():
                    for i in vals[3:]:
                        w.writeln('CarriageType::%s,' % types[i])
                w.writeln('},')
            w.writeln('},')
            data = data[10:]
    assert(len(data) == 0)
    w.writeln('};')
    w.writeln()
    w.writeln('} // namespace resl')
