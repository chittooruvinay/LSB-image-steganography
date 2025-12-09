🖼️ Image Steganography (LSB Technique) – C Project

A C-based implementation of Image Steganography using the Least Significant Bit (LSB) technique. This project hides a secret file inside a BMP image and retrieves it without visibly altering the carrier image.

📌 Features

Hide any file (.txt, .c, .sh, etc.) inside a BMP image
Extract hidden files from stego images
Strict input validation for file formats
Works at byte-level using raw BMP manipulation
Encodes metadata (magic string, extension, size)

🧠 How Encoding Works

Validate BMP & secret file
Read BMP header (first 54 bytes)
Calculate image capacity
Embed magic string (#*)

Embed:
Extension size
File extension
Secret file size
Actual secret data (bit by bit)
Save stego image

🧩 How Decoding Works
Load stego BMP
Skip header and extract bits from LSBs
Verify magic string
Decode extension size, extension, and file size
Reconstruct and save the hidden file

🛠️ Technologies Used

C programming
Bitwise operations
File handling

BMP image processing
