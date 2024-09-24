import serial
import time

# Inisialisasi serial port
port = 'COM14'  # Ganti sesuai dengan port yang digunakan, untuk Windows biasanya COMx, untuk Linux bisa /dev/ttySx atau /dev/ttyUSBx
baud_rate = 115200  # Sesuaikan dengan baud rate yang digunakan
timeout = 1  # Waktu timeout jika tidak ada data masuk (dalam detik)
log_file = 'serial_log.txt'  # Nama file log

def read_serial_and_log():
    try:
        # Buka koneksi serial
        with serial.Serial(port, baud_rate, timeout=timeout) as ser:
            print(f'Membaca dari {port} pada baud rate {baud_rate}')
            with open(log_file, 'a') as log:
                while True:
                    # Baca data dari serial
                    if ser.in_waiting > 0:
                        data = ser.readline().decode('utf-8').strip()
                        # Cetak data ke konsol dan simpan ke file log
                        print(data)
                        log.write(f"{time.strftime('%Y-%m-%d %H:%M:%S')} - {data}\n")
                    time.sleep(0.1)  # Beri jeda waktu kecil untuk tidak membebani CPU
    except serial.SerialException as e:
        print(f'Gagal mengakses serial port: {e}')
    except KeyboardInterrupt:
        print('Program dihentikan.')

if __name__ == "__main__":
    read_serial_and_log()
