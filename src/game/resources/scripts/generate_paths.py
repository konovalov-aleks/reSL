from cpp_writer import CPPWriter

import struct

# 1d7d:02d4 - 6594 bytes
data = b'\x98\x00\x00\x00\x04\x00\x01\x00\x04\x00\x02\x00\x04\x00\x02\x00\x04\x00\x02\x00\x04\x00\x03\x01\x04\x00\x03\x01\x04\x00\x04\x01\x04\x00\x05\x01\x04\x00\x06\x01\x04\x00\x06\x01\x04\x00\x06\x01\x04\x00\x07\x02\x04\x00\x07\x02\x04\x00\x08\x02\x04\x00\x09\x02\x04\x00\x0a\x02\x04\x00\x0a\x02\x04\x00\x0a\x02\x04\x00\x0b\x03\x04\x00\x0b\x03\x04\x00\x0c\x03\x04\x00\x0d\x03\x04\x00\x0e\x03\x04\x00\x0e\x03\x04\x00\x0e\x03\x04\x00\x0f\x04\x04\x00\x0f\x04\x04\x00\x10\x04\x04\x00\x11\x04\x04\x00\x12\x04\x04\x00\x12\x04\x04\x00\x12\x04\x04\x00\x13\x05\x04\x00\x13\x05\x04\x00\x14\x05\x04\x00\x15\x05\x04\x00\x16\x05\x04\x00\x16\x05\x04\x00\x16\x05\x04\x00\x17\x06\x04\x00\x17\x06\x04\x00\x18\x06\x04\x00\x19\x06\x04\x00\x1a\x06\x04\x00\x1b\x06\x04\x00\x1b\x06\x04\x00\x1b\x06\x04\x00\x1c\x07\x04\x00\x1c\x07\x04\x00\x1d\x07\x04\x00\x1e\x07\x04\x00\x1f\x07\x04\x00\x1f\x07\x04\x00\x1f\x07\x04\x00\x20\x08\x04\x00\x20\x08\x04\x00\x21\x08\x04\x00\x22\x08\x04\x00\x23\x08\x04\x00\x23\x08\x04\x00\x23\x08\x04\x00\x24\x09\x04\x00\x24\x09\x04\x00\x25\x09\x04\x00\x26\x09\x04\x00\x27\x09\x04\x00\x27\x09\x04\x00\x27\x09\x04\x00\x28\x0a\x04\x00\x28\x0a\x04\x00\x29\x0a\x04\x00\x2a\x0a\x04\x00\x2b\x0a\x04\x00\x2b\x0a\x04\x00\x2b\x0a\x04\x00\x2c\x0b\x04\x00\x2c\x0b\x04\x00\x2d\x0b\x04\x00\x2e\x0b\x04\x00\x2f\x0b\x04\x00\x30\x0b\x04\x00\x30\x0b\x04\x00\x30\x0b\x04\x00\x31\x0c\x04\x00\x31\x0c\x04\x00\x32\x0c\x04\x00\x33\x0c\x04\x00\x34\x0c\x04\x00\x34\x0c\x04\x00\x34\x0c\x04\x00\x35\x0d\x04\x00\x35\x0d\x04\x00\x36\x0d\x04\x00\x37\x0d\x04\x00\x38\x0d\x04\x00\x38\x0d\x04\x00\x38\x0d\x04\x00\x39\x0e\x04\x00\x39\x0e\x04\x00\x3a\x0e\x04\x00\x3b\x0e\x04\x00\x3c\x0e\x04\x00\x3c\x0e\x04\x00\x3c\x0e\x04\x00\x3d\x0f\x04\x00\x3d\x0f\x04\x00\x3e\x0f\x04\x00\x3f\x0f\x04\x00\x40\x0f\x04\x00\x40\x0f\x04\x00\x40\x0f\x04\x00\x41\x10\x04\x00\x41\x10\x04\x00\x42\x10\x04\x00\x43\x10\x04\x00\x44\x10\x04\x00\x45\x10\x04\x00\x45\x10\x04\x00\x45\x10\x04\x00\x46\x11\x04\x00\x46\x11\x04\x00\x47\x11\x04\x00\x48\x11\x04\x00\x49\x11\x04\x00\x49\x11\x04\x00\x49\x11\x04\x00\x4a\x12\x04\x00\x4a\x12\x04\x00\x4b\x12\x04\x00\x4c\x12\x04\x00\x4d\x12\x04\x00\x4d\x12\x04\x00\x4d\x12\x04\x00\x4e\x13\x04\x00\x4e\x13\x04\x00\x4f\x13\x04\x00\x50\x13\x04\x00\x51\x13\x04\x00\x51\x13\x04\x00\x51\x13\x04\x00\x52\x14\x04\x00\x52\x14\x04\x00\x53\x14\x04\x00\x54\x14\x04\x00\x55\x14\x04\x00\x55\x14\x04\x00\x55\x14\x04\x00\x56\x15\x04\x00\x56\x15\x04\x00\x57\x15\x04\x00\x58\x15\x04\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x98\x00\x00\x00\x01\x00\xff\x00\x01\x00\xfe\x00\x01\x00\xfe\x00\x01\x00\xfe\x00\x01\x00\xfd\x01\x01\x00\xfd\x01\x01\x00\xfc\x01\x01\x00\xfb\x01\x01\x00\xfa\x01\x01\x00\xfa\x01\x01\x00\xfa\x01\x01\x00\xf9\x02\x01\x00\xf9\x02\x01\x00\xf8\x02\x01\x00\xf7\x02\x01\x00\xf6\x02\x01\x00\xf6\x02\x01\x00\xf6\x02\x01\x00\xf5\x03\x01\x00\xf5\x03\x01\x00\xf4\x03\x01\x00\xf3\x03\x01\x00\xf2\x03\x01\x00\xf2\x03\x01\x00\xf2\x03\x01\x00\xf1\x04\x01\x00\xf1\x04\x01\x00\xf0\x04\x01\x00\xef\x04\x01\x00\xee\x04\x01\x00\xee\x04\x01\x00\xee\x04\x01\x00\xed\x05\x01\x00\xed\x05\x01\x00\xec\x05\x01\x00\xeb\x05\x01\x00\xea\x05\x01\x00\xea\x05\x01\x00\xea\x05\x01\x00\xe9\x06\x01\x00\xe9\x06\x01\x00\xe8\x06\x01\x00\xe7\x06\x01\x00\xe6\x06\x01\x00\xe5\x06\x01\x00\xe5\x06\x01\x00\xe5\x06\x01\x00\xe4\x07\x01\x00\xe4\x07\x01\x00\xe3\x07\x01\x00\xe2\x07\x01\x00\xe1\x07\x01\x00\xe1\x07\x01\x00\xe1\x07\x01\x00\xe0\x08\x01\x00\xe0\x08\x01\x00\xdf\x08\x01\x00\xde\x08\x01\x00\xdd\x08\x01\x00\xdd\x08\x01\x00\xdd\x08\x01\x00\xdc\x09\x01\x00\xdc\x09\x01\x00\xdb\x09\x01\x00\xda\x09\x01\x00\xd9\x09\x01\x00\xd9\x09\x01\x00\xd9\x09\x01\x00\xd8\x0a\x01\x00\xd8\x0a\x01\x00\xd7\x0a\x01\x00\xd6\x0a\x01\x00\xd5\x0a\x01\x00\xd5\x0a\x01\x00\xd5\x0a\x01\x00\xd4\x0b\x01\x00\xd4\x0b\x01\x00\xd3\x0b\x01\x00\xd2\x0b\x01\x00\xd1\x0b\x01\x00\xd0\x0b\x01\x00\xd0\x0b\x01\x00\xd0\x0b\x01\x00\xcf\x0c\x01\x00\xcf\x0c\x01\x00\xce\x0c\x01\x00\xcd\x0c\x01\x00\xcc\x0c\x01\x00\xcc\x0c\x01\x00\xcc\x0c\x01\x00\xcb\x0d\x01\x00\xcb\x0d\x01\x00\xca\x0d\x01\x00\xc9\x0d\x01\x00\xc8\x0d\x01\x00\xc8\x0d\x01\x00\xc8\x0d\x01\x00\xc7\x0e\x01\x00\xc7\x0e\x01\x00\xc6\x0e\x01\x00\xc5\x0e\x01\x00\xc4\x0e\x01\x00\xc4\x0e\x01\x00\xc4\x0e\x01\x00\xc3\x0f\x01\x00\xc3\x0f\x01\x00\xc2\x0f\x01\x00\xc1\x0f\x01\x00\xc0\x0f\x01\x00\xc0\x0f\x01\x00\xc0\x0f\x01\x00\xbf\x10\x01\x00\xbf\x10\x01\x00\xbe\x10\x01\x00\xbd\x10\x01\x00\xbc\x10\x01\x00\xbb\x10\x01\x00\xbb\x10\x01\x00\xbb\x10\x01\x00\xba\x11\x01\x00\xba\x11\x01\x00\xb9\x11\x01\x00\xb8\x11\x01\x00\xb7\x11\x01\x00\xb7\x11\x01\x00\xb7\x11\x01\x00\xb6\x12\x01\x00\xb6\x12\x01\x00\xb5\x12\x01\x00\xb4\x12\x01\x00\xb3\x12\x01\x00\xb3\x12\x01\x00\xb3\x12\x01\x00\xb2\x13\x01\x00\xb2\x13\x01\x00\xb1\x13\x01\x00\xb0\x13\x01\x00\xaf\x13\x01\x00\xaf\x13\x01\x00\xaf\x13\x01\x00\xae\x14\x01\x00\xae\x14\x01\x00\xad\x14\x01\x00\xac\x14\x01\x00\xab\x14\x01\x00\xab\x14\x01\x00\xab\x14\x01\x00\xaa\x15\x01\x00\xaa\x15\x01\x00\xa9\x15\x01\x00\xa8\x15\x01\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\xcf\x00\x00\x00\x04\x00\x01\x00\x04\x00\x02\x00\x04\x00\x02\x00\x04\x00\x02\x00\x04\x00\x03\x01\x04\x00\x03\x01\x04\x00\x04\x01\x04\x00\x05\x01\x04\x00\x05\x01\x04\x00\x05\x01\x04\x00\x06\x02\x04\x00\x06\x02\x04\x00\x07\x02\x04\x00\x08\x02\x04\x00\x09\x02\x04\x00\x09\x02\x04\x00\x09\x02\x04\x00\x0a\x03\x04\x00\x0a\x03\x04\x00\x0b\x03\x04\x00\x0c\x03\x04\x00\x0c\x03\x04\x00\x0c\x03\x04\x00\x0d\x04\x04\x00\x0d\x04\x04\x00\x0e\x04\x04\x00\x0f\x04\x04\x00\x0f\x04\x04\x00\x0f\x04\x04\x00\x10\x05\x04\x00\x10\x05\x04\x00\x11\x05\x04\x00\x11\x05\x04\x00\x11\x05\x04\x00\x12\x06\x04\x00\x12\x06\x04\x00\x13\x06\x04\x00\x14\x06\x04\x00\x14\x06\x04\x00\x14\x06\x04\x00\x15\x07\x03\x00\x15\x07\x03\x00\x16\x07\x03\x00\x16\x07\x03\x00\x16\x07\x03\x00\x17\x08\x03\x00\x17\x08\x03\x00\x18\x08\x03\x00\x18\x08\x03\x00\x18\x08\x03\x00\x19\x09\x03\x00\x19\x09\x03\x00\x1a\x09\x03\x00\x1a\x09\x03\x00\x1a\x09\x03\x00\x1b\x0a\x03\x00\x1b\x0a\x03\x00\x1c\x0a\x03\x00\x1c\x0a\x03\x00\x1c\x0a\x03\x00\x1d\x0b\x03\x00\x1d\x0b\x03\x00\x1d\x0b\x03\x00\x1d\x0b\x03\x00\x1e\x0c\x03\x00\x1e\x0c\x03\x00\x1f\x0c\x03\x00\x1f\x0c\x03\x00\x1f\x0c\x03\x00\x20\x0d\x03\x00\x20\x0d\x03\x00\x20\x0d\x03\x00\x20\x0d\x03\x00\x21\x0e\x03\x00\x21\x0e\x03\x00\x21\x0e\x03\x00\x21\x0e\x03\x00\x22\x0f\x03\x00\x22\x0f\x03\x00\x22\x0f\x03\x00\x22\x0f\x03\x00\x22\x10\x03\x00\x22\x10\x03\x00\x22\x10\x03\x00\x22\x10\x03\x00\x23\x11\x03\x00\x23\x11\x03\x00\x23\x11\x03\x00\x23\x11\x03\x00\x24\x12\x03\x00\x24\x12\x03\x00\x24\x12\x03\x00\x24\x12\x03\x00\x24\x13\x03\x00\x24\x13\x03\x00\x24\x13\x03\x00\x24\x13\x03\x00\x25\x14\x03\x00\x25\x14\x03\x00\x25\x14\x03\x00\x25\x14\x03\x00\x25\x15\x03\x00\x25\x15\x02\x00\x25\x15\x02\x00\x25\x15\x02\x00\x25\x16\x02\x00\x25\x16\x02\x00\x25\x16\x02\x00\x25\x16\x02\x00\x24\x17\x02\x00\x24\x17\x02\x00\x24\x17\x02\x00\x24\x17\x02\x00\x24\x18\x02\x00\x24\x18\x02\x00\x24\x18\x02\x00\x24\x18\x02\x00\x23\x19\x02\x00\x23\x19\x02\x00\x23\x19\x02\x00\x23\x19\x02\x00\x22\x1a\x02\x00\x22\x1a\x02\x00\x22\x1a\x02\x00\x22\x1a\x02\x00\x22\x1b\x02\x00\x22\x1b\x02\x00\x22\x1b\x02\x00\x22\x1b\x02\x00\x21\x1c\x02\x00\x21\x1c\x02\x00\x21\x1c\x02\x00\x21\x1c\x02\x00\x20\x1d\x02\x00\x20\x1d\x02\x00\x20\x1d\x02\x00\x20\x1d\x02\x00\x1f\x1e\x02\x00\x1f\x1e\x02\x00\x1e\x1e\x02\x00\x1e\x1e\x02\x00\x1e\x1e\x02\x00\x1d\x1f\x02\x00\x1d\x1f\x02\x00\x1d\x1f\x02\x00\x1d\x1f\x02\x00\x1c\x20\x02\x00\x1c\x20\x02\x00\x1b\x20\x02\x00\x1b\x20\x02\x00\x1b\x20\x02\x00\x1a\x21\x02\x00\x1a\x21\x02\x00\x19\x21\x02\x00\x19\x21\x02\x00\x19\x21\x02\x00\x18\x22\x02\x00\x18\x22\x02\x00\x17\x22\x02\x00\x17\x22\x02\x00\x17\x22\x02\x00\x16\x23\x02\x00\x16\x23\x02\x00\x15\x23\x01\x00\x15\x23\x01\x00\x15\x23\x01\x00\x14\x24\x01\x00\x14\x24\x01\x00\x13\x24\x01\x00\x12\x24\x01\x00\x12\x24\x01\x00\x12\x24\x01\x00\x11\x25\x01\x00\x11\x25\x01\x00\x10\x25\x01\x00\x10\x25\x01\x00\x10\x25\x01\x00\x0f\x26\x01\x00\x0f\x26\x01\x00\x0e\x26\x01\x00\x0d\x26\x01\x00\x0d\x26\x01\x00\x0d\x26\x01\x00\x0c\x27\x01\x00\x0c\x27\x01\x00\x0b\x27\x01\x00\x0a\x27\x01\x00\x0a\x27\x01\x00\x0a\x27\x01\x00\x09\x28\x01\x00\x09\x28\x01\x00\x08\x28\x01\x00\x07\x28\x01\x00\x06\x28\x01\x00\x06\x28\x01\x00\x06\x28\x01\x00\x05\x29\x01\x00\x05\x29\x01\x00\x04\x29\x01\x00\x03\x29\x01\x00\x03\x29\x01\x00\x03\x29\x01\x00\x02\x2a\x01\x00\x02\x2a\x01\x00\x01\x2a\x01\x00\x00\x2a\x01\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\xe7\x00\x58\x15\x01\x00\x57\x15\x01\x00\x56\x15\x01\x00\x56\x15\x01\x00\x56\x15\x01\x00\x55\x16\x01\x00\x55\x16\x01\x00\x54\x16\x01\x00\x53\x16\x01\x00\x52\x16\x01\x00\x52\x16\x01\x00\x52\x16\x01\x00\x51\x17\x01\x00\x51\x17\x01\x00\x50\x17\x01\x00\x4f\x17\x01\x00\x4e\x17\x01\x00\x4d\x17\x01\x00\x4d\x17\x01\x00\x4d\x17\x01\x00\x4c\x18\x01\x00\x4c\x18\x01\x00\x4b\x18\x01\x00\x4a\x18\x01\x00\x49\x18\x01\x00\x48\x18\x01\x00\x47\x18\x01\x00\x47\x18\x01\x00\x47\x18\x01\x00\x46\x19\x01\x00\x46\x19\x01\x00\x45\x19\x01\x00\x44\x19\x01\x00\x43\x19\x01\x00\x42\x19\x01\x00\x41\x19\x01\x00\x41\x19\x01\x00\x41\x19\x01\x00\x40\x1a\x01\x00\x40\x1a\x01\x00\x3f\x1a\x01\x00\x3e\x1a\x01\x00\x3d\x1a\x01\x00\x3c\x1a\x01\x00\x3b\x1a\x01\x00\x3a\x1a\x01\x00\x39\x1a\x01\x00\x39\x1a\x01\x00\x39\x1a\x01\x00\x38\x1b\x01\x00\x38\x1b\x01\x00\x37\x1b\x01\x00\x36\x1b\x01\x00\x35\x1b\x01\x00\x34\x1b\x01\x00\x33\x1b\x01\x00\x32\x1b\x00\x00\x31\x1b\x00\x00\x30\x1b\x00\x00\x2f\x1b\x00\x00\x2f\x1b\x00\x00\x2f\x1b\x00\x00\x2e\x1c\x00\x00\x2e\x1c\x00\x00\x2d\x1c\x00\x00\x2c\x1c\x00\x00\x2b\x1c\x00\x00\x2a\x1c\x00\x00\x29\x1c\x00\x00\x28\x1c\x00\x00\x27\x1c\x00\x00\x26\x1c\x00\x00\x25\x1c\x00\x00\x24\x1c\x00\x00\x23\x1c\x00\x00\x23\x1c\x00\x00\x23\x1c\x00\x00\x22\x1d\x00\x00\x22\x1d\x00\x00\x21\x1d\x00\x00\x20\x1d\x00\x00\x1f\x1d\x00\x00\x1e\x1d\x00\x00\x1d\x1d\x00\x00\x1c\x1d\x00\x00\x1b\x1d\x00\x00\x1a\x1d\x00\x00\x19\x1d\x00\x00\x18\x1d\x00\x00\x17\x1d\x00\x00\x16\x1d\x00\x00\x15\x1d\x00\x00\x14\x1d\x00\x00\x13\x1d\x00\x00\x12\x1d\x00\x00\x12\x1d\x00\x00\x12\x1d\x00\x00\x11\x1e\x00\x00\x11\x1e\x00\x00\x10\x1e\x00\x00\x0f\x1e\x00\x00\x0e\x1e\x00\x00\x0d\x1e\x00\x00\x0c\x1e\x00\x00\x0b\x1e\x00\x00\x0a\x1e\x00\x00\x09\x1e\x00\x00\x08\x1e\x00\x00\x07\x1e\x00\x00\x06\x1e\x00\x00\x05\x1e\x00\x00\x04\x1e\x00\x00\x03\x1e\x00\x00\x02\x1e\x00\x00\x01\x1e\x00\x00\x00\x1e\x00\x00\xff\x1e\x00\x00\xfe\x1e\x00\x00\xfd\x1e\x00\x00\xfc\x1e\x00\x00\xfb\x1e\x00\x00\xfa\x1e\x00\x00\xf9\x1e\x00\x00\xf8\x1e\x00\x00\xf7\x1e\x00\x00\xf6\x1e\x00\x00\xf5\x1e\x00\x00\xf4\x1e\x00\x00\xf3\x1e\x00\x00\xf2\x1e\x00\x00\xf1\x1e\x00\x00\xf0\x1e\x00\x00\xef\x1e\x00\x00\xef\x1e\x00\x00\xef\x1e\x00\x00\xee\x1d\x00\x00\xee\x1d\x00\x00\xed\x1d\x00\x00\xec\x1d\x00\x00\xeb\x1d\x00\x00\xea\x1d\x00\x00\xe9\x1d\x00\x00\xe8\x1d\x00\x00\xe7\x1d\x00\x00\xe6\x1d\x00\x00\xe5\x1d\x00\x00\xe4\x1d\x00\x00\xe3\x1d\x00\x00\xe2\x1d\x00\x00\xe1\x1d\x00\x00\xe0\x1d\x00\x00\xdf\x1d\x00\x00\xde\x1d\x00\x00\xde\x1d\x00\x00\xde\x1d\x00\x00\xdd\x1c\x00\x00\xdd\x1c\x00\x00\xdc\x1c\x00\x00\xdb\x1c\x00\x00\xda\x1c\x00\x00\xd9\x1c\x00\x00\xd8\x1c\x00\x00\xd7\x1c\x00\x00\xd6\x1c\x00\x00\xd5\x1c\x00\x00\xd4\x1c\x00\x00\xd3\x1c\x00\x00\xd2\x1c\x00\x00\xd2\x1c\x00\x00\xd2\x1c\x00\x00\xd1\x1b\x00\x00\xd1\x1b\x00\x00\xd0\x1b\x00\x00\xcf\x1b\x00\x00\xce\x1b\x00\x00\xcd\x1b\x04\x00\xcc\x1b\x04\x00\xcb\x1b\x04\x00\xca\x1b\x04\x00\xc9\x1b\x04\x00\xc8\x1b\x04\x00\xc8\x1b\x04\x00\xc8\x1b\x04\x00\xc7\x1a\x04\x00\xc7\x1a\x04\x00\xc6\x1a\x04\x00\xc5\x1a\x04\x00\xc4\x1a\x04\x00\xc3\x1a\x04\x00\xc2\x1a\x04\x00\xc1\x1a\x04\x00\xc0\x1a\x04\x00\xc0\x1a\x04\x00\xc0\x1a\x04\x00\xbf\x19\x04\x00\xbf\x19\x04\x00\xbe\x19\x04\x00\xbd\x19\x04\x00\xbc\x19\x04\x00\xbb\x19\x04\x00\xba\x19\x04\x00\xba\x19\x04\x00\xba\x19\x04\x00\xb9\x18\x04\x00\xb9\x18\x04\x00\xb8\x18\x04\x00\xb7\x18\x04\x00\xb6\x18\x04\x00\xb5\x18\x04\x00\xb4\x18\x04\x00\xb4\x18\x04\x00\xb4\x18\x04\x00\xb3\x17\x04\x00\xb3\x17\x04\x00\xb2\x17\x04\x00\xb1\x17\x04\x00\xb0\x17\x04\x00\xaf\x17\x04\x00\xaf\x17\x04\x00\xaf\x17\x04\x00\xae\x16\x04\x00\xae\x16\x04\x00\xad\x16\x04\x00\xac\x16\x04\x00\xab\x16\x04\x00\xab\x16\x04\x00\xab\x16\x04\x00\xaa\x15\x04\x00\xaa\x15\x04\x00\xa9\x15\x04\x00\xa8\x15\x04\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\xcf\x00\x00\x00\x01\x00\xff\x00\x01\x00\xfe\x00\x01\x00\xfe\x00\x01\x00\xfe\x00\x01\x00\xfd\x01\x01\x00\xfd\x01\x01\x00\xfc\x01\x01\x00\xfb\x01\x01\x00\xfb\x01\x01\x00\xfb\x01\x01\x00\xfa\x02\x01\x00\xfa\x02\x01\x00\xf9\x02\x01\x00\xf8\x02\x01\x00\xf7\x02\x01\x00\xf7\x02\x01\x00\xf7\x02\x01\x00\xf6\x03\x01\x00\xf6\x03\x01\x00\xf5\x03\x01\x00\xf4\x03\x01\x00\xf4\x03\x01\x00\xf4\x03\x01\x00\xf3\x04\x01\x00\xf3\x04\x01\x00\xf2\x04\x01\x00\xf1\x04\x01\x00\xf1\x04\x01\x00\xf1\x04\x01\x00\xf0\x05\x01\x00\xf0\x05\x01\x00\xef\x05\x01\x00\xef\x05\x01\x00\xef\x05\x01\x00\xee\x06\x01\x00\xee\x06\x01\x00\xed\x06\x01\x00\xec\x06\x01\x00\xec\x06\x01\x00\xec\x06\x01\x00\xeb\x07\x01\x00\xeb\x07\x01\x00\xea\x07\x01\x00\xea\x07\x01\x00\xea\x07\x01\x00\xe9\x08\x02\x00\xe9\x08\x02\x00\xe8\x08\x02\x00\xe8\x08\x02\x00\xe8\x08\x02\x00\xe7\x09\x02\x00\xe7\x09\x02\x00\xe6\x09\x02\x00\xe6\x09\x02\x00\xe6\x09\x02\x00\xe5\x0a\x02\x00\xe5\x0a\x02\x00\xe4\x0a\x02\x00\xe4\x0a\x02\x00\xe4\x0a\x02\x00\xe3\x0b\x02\x00\xe3\x0b\x02\x00\xe3\x0b\x02\x00\xe3\x0b\x02\x00\xe2\x0c\x02\x00\xe2\x0c\x02\x00\xe1\x0c\x02\x00\xe1\x0c\x02\x00\xe1\x0c\x02\x00\xe0\x0d\x02\x00\xe0\x0d\x02\x00\xe0\x0d\x02\x00\xe0\x0d\x02\x00\xdf\x0e\x02\x00\xdf\x0e\x02\x00\xdf\x0e\x02\x00\xdf\x0e\x02\x00\xde\x0f\x02\x00\xde\x0f\x02\x00\xde\x0f\x02\x00\xde\x0f\x02\x00\xde\x10\x02\x00\xde\x10\x02\x00\xde\x10\x02\x00\xde\x10\x02\x00\xdd\x11\x02\x00\xdd\x11\x02\x00\xdd\x11\x02\x00\xdd\x11\x02\x00\xdc\x12\x02\x00\xdc\x12\x02\x00\xdc\x12\x02\x00\xdc\x12\x02\x00\xdc\x13\x02\x00\xdc\x13\x02\x00\xdc\x13\x02\x00\xdc\x13\x02\x00\xdb\x14\x02\x00\xdb\x14\x02\x00\xdb\x14\x02\x00\xdb\x14\x02\x00\xdb\x15\x02\x00\xdb\x15\x03\x00\xdb\x15\x03\x00\xdb\x15\x03\x00\xdb\x16\x03\x00\xdb\x16\x03\x00\xdb\x16\x03\x00\xdb\x16\x03\x00\xdc\x17\x03\x00\xdc\x17\x03\x00\xdc\x17\x03\x00\xdc\x17\x03\x00\xdc\x18\x03\x00\xdc\x18\x03\x00\xdc\x18\x03\x00\xdc\x18\x03\x00\xdd\x19\x03\x00\xdd\x19\x03\x00\xdd\x19\x03\x00\xdd\x19\x03\x00\xde\x1a\x03\x00\xde\x1a\x03\x00\xde\x1a\x03\x00\xde\x1a\x03\x00\xde\x1b\x03\x00\xde\x1b\x03\x00\xde\x1b\x03\x00\xde\x1b\x03\x00\xdf\x1c\x03\x00\xdf\x1c\x03\x00\xdf\x1c\x03\x00\xdf\x1c\x03\x00\xe0\x1d\x03\x00\xe0\x1d\x03\x00\xe0\x1d\x03\x00\xe0\x1d\x03\x00\xe1\x1e\x03\x00\xe1\x1e\x03\x00\xe2\x1e\x03\x00\xe2\x1e\x03\x00\xe2\x1e\x03\x00\xe3\x1f\x03\x00\xe3\x1f\x03\x00\xe3\x1f\x03\x00\xe3\x1f\x03\x00\xe4\x20\x03\x00\xe4\x20\x03\x00\xe5\x20\x03\x00\xe5\x20\x03\x00\xe5\x20\x03\x00\xe6\x21\x03\x00\xe6\x21\x03\x00\xe7\x21\x03\x00\xe7\x21\x03\x00\xe7\x21\x03\x00\xe8\x22\x03\x00\xe8\x22\x03\x00\xe9\x22\x03\x00\xe9\x22\x03\x00\xe9\x22\x03\x00\xea\x23\x03\x00\xea\x23\x03\x00\xeb\x23\x04\x00\xeb\x23\x04\x00\xeb\x23\x04\x00\xec\x24\x04\x00\xec\x24\x04\x00\xed\x24\x04\x00\xee\x24\x04\x00\xee\x24\x04\x00\xee\x24\x04\x00\xef\x25\x04\x00\xef\x25\x04\x00\xf0\x25\x04\x00\xf0\x25\x04\x00\xf0\x25\x04\x00\xf1\x26\x04\x00\xf1\x26\x04\x00\xf2\x26\x04\x00\xf3\x26\x04\x00\xf3\x26\x04\x00\xf3\x26\x04\x00\xf4\x27\x04\x00\xf4\x27\x04\x00\xf5\x27\x04\x00\xf6\x27\x04\x00\xf6\x27\x04\x00\xf6\x27\x04\x00\xf7\x28\x04\x00\xf7\x28\x04\x00\xf8\x28\x04\x00\xf9\x28\x04\x00\xfa\x28\x04\x00\xfa\x28\x04\x00\xfa\x28\x04\x00\xfb\x29\x04\x00\xfb\x29\x04\x00\xfc\x29\x04\x00\xfd\x29\x04\x00\xfd\x29\x04\x00\xfd\x29\x04\x00\xfe\x2a\x04\x00\xfe\x2a\x04\x00\xff\x2a\x04\x00\x00\x2a\x04\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\xe7\x00\x58\x15\x04\x00\x57\x15\x04\x00\x56\x15\x04\x00\x56\x15\x04\x00\x56\x15\x04\x00\x55\x14\x04\x00\x55\x14\x04\x00\x54\x14\x04\x00\x53\x14\x04\x00\x52\x14\x04\x00\x52\x14\x04\x00\x52\x14\x04\x00\x51\x13\x04\x00\x51\x13\x04\x00\x50\x13\x04\x00\x4f\x13\x04\x00\x4e\x13\x04\x00\x4d\x13\x04\x00\x4d\x13\x04\x00\x4d\x13\x04\x00\x4c\x12\x04\x00\x4c\x12\x04\x00\x4b\x12\x04\x00\x4a\x12\x04\x00\x49\x12\x04\x00\x48\x12\x04\x00\x47\x12\x04\x00\x47\x12\x04\x00\x47\x12\x04\x00\x46\x11\x04\x00\x46\x11\x04\x00\x45\x11\x04\x00\x44\x11\x04\x00\x43\x11\x04\x00\x42\x11\x04\x00\x41\x11\x04\x00\x41\x11\x04\x00\x41\x11\x04\x00\x40\x10\x04\x00\x40\x10\x04\x00\x3f\x10\x04\x00\x3e\x10\x04\x00\x3d\x10\x04\x00\x3c\x10\x04\x00\x3b\x10\x04\x00\x3a\x10\x04\x00\x39\x10\x04\x00\x39\x10\x04\x00\x39\x10\x04\x00\x38\x0f\x04\x00\x38\x0f\x04\x00\x37\x0f\x04\x00\x36\x0f\x04\x00\x35\x0f\x04\x00\x34\x0f\x04\x00\x33\x0f\x04\x00\x32\x0f\x00\x00\x31\x0f\x00\x00\x30\x0f\x00\x00\x2f\x0f\x00\x00\x2f\x0f\x00\x00\x2f\x0f\x00\x00\x2e\x0e\x00\x00\x2e\x0e\x00\x00\x2d\x0e\x00\x00\x2c\x0e\x00\x00\x2b\x0e\x00\x00\x2a\x0e\x00\x00\x29\x0e\x00\x00\x28\x0e\x00\x00\x27\x0e\x00\x00\x26\x0e\x00\x00\x25\x0e\x00\x00\x24\x0e\x00\x00\x23\x0e\x00\x00\x23\x0e\x00\x00\x23\x0e\x00\x00\x22\x0d\x00\x00\x22\x0d\x00\x00\x21\x0d\x00\x00\x20\x0d\x00\x00\x1f\x0d\x00\x00\x1e\x0d\x00\x00\x1d\x0d\x00\x00\x1c\x0d\x00\x00\x1b\x0d\x00\x00\x1a\x0d\x00\x00\x19\x0d\x00\x00\x18\x0d\x00\x00\x17\x0d\x00\x00\x16\x0d\x00\x00\x15\x0d\x00\x00\x14\x0d\x00\x00\x13\x0d\x00\x00\x12\x0d\x00\x00\x12\x0d\x00\x00\x12\x0d\x00\x00\x11\x0d\x00\x00\x11\x0d\x00\x00\x10\x0d\x00\x00\x0f\x0d\x00\x00\x0e\x0d\x00\x00\x0d\x0d\x00\x00\x0c\x0d\x00\x00\x0b\x0d\x00\x00\x0a\x0d\x00\x00\x09\x0d\x00\x00\x08\x0d\x00\x00\x07\x0d\x00\x00\x06\x0d\x00\x00\x05\x0d\x00\x00\x04\x0d\x00\x00\x03\x0d\x00\x00\x02\x0d\x00\x00\x01\x0d\x00\x00\x00\x0d\x00\x00\xff\x0d\x00\x00\xfe\x0d\x00\x00\xfd\x0d\x00\x00\xfc\x0d\x00\x00\xfb\x0d\x00\x00\xfa\x0d\x00\x00\xf9\x0d\x00\x00\xf8\x0d\x00\x00\xf7\x0d\x00\x00\xf6\x0d\x00\x00\xf5\x0d\x00\x00\xf4\x0d\x00\x00\xf3\x0d\x00\x00\xf2\x0d\x00\x00\xf1\x0d\x00\x00\xf0\x0d\x00\x00\xef\x0d\x00\x00\xef\x0d\x00\x00\xef\x0d\x00\x00\xee\x0d\x00\x00\xee\x0d\x00\x00\xed\x0d\x00\x00\xec\x0d\x00\x00\xeb\x0d\x00\x00\xea\x0d\x00\x00\xe9\x0d\x00\x00\xe8\x0d\x00\x00\xe7\x0d\x00\x00\xe6\x0d\x00\x00\xe5\x0d\x00\x00\xe4\x0d\x00\x00\xe3\x0d\x00\x00\xe2\x0d\x00\x00\xe1\x0d\x00\x00\xe0\x0d\x00\x00\xdf\x0d\x00\x00\xde\x0d\x00\x00\xde\x0d\x00\x00\xde\x0d\x00\x00\xdd\x0e\x00\x00\xdd\x0e\x00\x00\xdc\x0e\x00\x00\xdb\x0e\x00\x00\xda\x0e\x00\x00\xd9\x0e\x00\x00\xd8\x0e\x00\x00\xd7\x0e\x00\x00\xd6\x0e\x00\x00\xd5\x0e\x00\x00\xd4\x0e\x00\x00\xd3\x0e\x00\x00\xd2\x0e\x00\x00\xd2\x0e\x00\x00\xd2\x0e\x00\x00\xd1\x0f\x00\x00\xd1\x0f\x00\x00\xd0\x0f\x00\x00\xcf\x0f\x00\x00\xce\x0f\x00\x00\xcd\x0f\x00\x00\xcc\x0f\x00\x00\xcb\x0f\x00\x00\xca\x0f\x00\x00\xc9\x0f\x00\x00\xc8\x0f\x01\x00\xc8\x0f\x01\x00\xc8\x0f\x01\x00\xc7\x10\x01\x00\xc7\x10\x01\x00\xc6\x10\x01\x00\xc5\x10\x01\x00\xc4\x10\x01\x00\xc3\x10\x01\x00\xc2\x10\x01\x00\xc1\x10\x01\x00\xc0\x10\x01\x00\xc0\x10\x01\x00\xc0\x10\x01\x00\xbf\x11\x01\x00\xbf\x11\x01\x00\xbe\x11\x01\x00\xbd\x11\x01\x00\xbc\x11\x01\x00\xbb\x11\x01\x00\xba\x11\x01\x00\xba\x11\x01\x00\xba\x11\x01\x00\xb9\x12\x01\x00\xb9\x12\x01\x00\xb8\x12\x01\x00\xb7\x12\x01\x00\xb6\x12\x01\x00\xb5\x12\x01\x00\xb4\x12\x01\x00\xb4\x12\x01\x00\xb4\x12\x01\x00\xb3\x13\x01\x00\xb3\x13\x01\x00\xb2\x13\x01\x00\xb1\x13\x01\x00\xb0\x13\x01\x00\xaf\x13\x01\x00\xaf\x13\x01\x00\xaf\x13\x01\x00\xae\x14\x01\x00\xae\x14\x01\x00\xad\x14\x01\x00\xac\x14\x01\x00\xab\x14\x01\x00\xab\x14\x01\x00\xab\x14\x01\x00\xaa\x15\x01\x00\xaa\x15\x01\x00\xa9\x15\x01\x00\xa8\x15\x01\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\xff\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00'

with CPPWriter('movement_paths.cpp') as w:
    w.writeln('#include "movement_paths.h"')
    w.writeln()
    w.writeln('namespace resl {')
    w.writeln()
    w.writeln('/* 1d7d:02d4 - 6594 bytes */')
    w.writeln('const Path g_movementPaths[7] = {')
    with w.indent():
        for i in range(7):
            w.writeln('{')
            with w.indent():
                size = int.from_bytes(data[:2], 'little')
                data = data[2:]
                w.writeln('%d,' % size)
                w.writeln('{');
                with w.indent():
                    if size != 255:
                        for j in range(size):
                            values = struct.unpack('<bbBB', data[j * 4: (j + 1) * 4])
                            w.writeWithWrap("{ %d, %d, %d }, " % values[:-1])
                    data = data[940:]
                w.writeln('}')
            w.writeln('}' if i == 6 else '},')
    w.writeln('};')
    w.writeln()
    w.writeln('} // namespace resl')