# Program Manajemen Proyek

Program ini adalah sistem manajemen proyek sederhana yang memungkinkan pengguna untuk mengelola proyek dan tugas-tugasnya.

## Kompilasi Program

### Menggunakan GCC (Windows/Linux/Mac)
```bash
# Kompilasi dengan peringatan dan informasi debug
gcc -Wall -Wextra -g proyek.c -o proyek
```

### Menggunakan Make (jika Makefile tersedia)
```bash
# Kompilasi menggunakan Make
make

# Membersihkan file hasil kompilasi
make clean
```

## Menjalankan Program

### Windows
```bash
# Menggunakan PowerShell
.\proyek

# Menggunakan Command Prompt
proyek.exe
```

### Linux/Mac
```bash
./proyek
```

## Fitur Program
1. Manajemen Proyek
   - Membuat proyek baru
   - Mengedit detail proyek
   - Menghapus proyek
   - Melihat daftar proyek

2. Manajemen Tugas
   - Membuat tugas baru
   - Menambah sub-tugas
   - Mengedit detail tugas
   - Menghapus tugas
   - Melihat struktur tugas (WBS)

3. Pencarian
   - Mencari proyek berdasarkan nama
   - Mencari tugas dalam proyek

4. Laporan
   - Laporan tugas berdasarkan status
   - Laporan tugas berdasarkan prioritas

5. Fitur Tambahan
   - Undo pembuatan tugas
   - Batch delete tugas
   - Penyimpanan dan pemuatan data

## Catatan
- Program akan menyimpan data secara otomatis ke file `project_data.txt`
- Data akan dimuat secara otomatis saat program dijalankan
- Pastikan file `project_data.txt` memiliki izin baca/tulis yang sesuai 