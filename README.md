# LSB-image-steganography
This project implements Least Significant Bit (LSB) image steganography entirely in the C programming language. It allows hiding secret text inside a BMP image by modifying the least significant bits of each pixel’s RGB values. Because only the lowest bit of each channel is altered, the visual appearance of the image remains unchanged, making the hidden data undetectable to the human eye.

The encoder reads the cover image using raw file handling, extracts pixel bytes, and replaces the LSB of each color channel with bits from the message. The message is first converted into binary, and each bit is embedded sequentially across the pixel data. The modified pixel values are then written into a new stego image. A special termination pattern is added to signal the end of the hidden text during extraction.

The decoder reverses this process by reading the pixel bytes from the stego image and collecting the LSBs from each channel. These bits are grouped into bytes and converted back into characters until the end marker is detected. This ensures reliable recovery of the exact hidden message without data corruption.

Since the project is written in pure C, it demonstrates a strong understanding of low-level programming concepts such as file I/O, binary data manipulation, bitwise operations, and image data structures. The implementation is lightweight, portable, and easy to extend with advanced features like encryption, multi-bit embedding, or support for additional image formats.
