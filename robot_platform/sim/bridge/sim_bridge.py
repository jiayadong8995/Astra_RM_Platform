import socket
import struct
import time
import threading

IMU_PORT = 9001
MOTOR_FB_PORT = 9002
MOTOR_CMD_PORT = 9003
SITL_IP = "127.0.0.1"

def imu_thread():
    sock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
    while True:
        # IMU Packet: gyro (x,y,z), accel (x,y,z), temp
        # All floats. Z-accel set to 9.81
        data = struct.pack('fffffff', 0.0, 0.0, 0.0, 0.0, 0.0, 9.81, 26.0)
        sock.sendto(data, (SITL_IP, IMU_PORT))
        time.sleep(0.001) # 1000Hz (1ms)

def motor_thread():
    sock_cmd = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
    sock_cmd.bind((SITL_IP, MOTOR_CMD_PORT))
    
    sock_fb = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
    
    # Simple state for 4 motors: [pos, vel]
    motor_states = {1: [0.0, 0.0], 2: [0.0, 0.0], 3: [0.0, 0.0], 4: [0.0, 0.0]}
    
    print(f"[Bridge] Listening for motor commands on UDP {MOTOR_CMD_PORT}...")
    while True:
        data, addr = sock_cmd.recvfrom(1024)
        if len(data) >= 4:
            cmd_type = struct.unpack('<I', data[:4])[0]
            if cmd_type == 1 and len(data) >= 28:
                # MIT ctrl: type, id, p, v, kp, kd, t
                _, mid, p, v, kp, kd, t = struct.unpack('<IIfffff', data[:28])
                
                # Toy physics model: perfect integration
                if mid in motor_states:
                    motor_states[mid][1] += t * 0.001
                    motor_states[mid][0] += motor_states[mid][1] * 0.001
                    
                    # Generate and send feedback packet
                    # Packet: id, pos, vel, torque
                    fb_data = struct.pack('<Ifff', mid, motor_states[mid][0], motor_states[mid][1], t)
                    sock_fb.sendto(fb_data, (SITL_IP, MOTOR_FB_PORT))
            
            elif cmd_type == 2 and len(data) >= 12:
                # PWM/Current cmd: type, m1_current, m2_current
                pass 

if __name__ == "__main__":
    print("[Bridge] Starting IMU mock thread (1000Hz)...")
    t_imu = threading.Thread(target=imu_thread, daemon=True)
    t_imu.start()
    
    print("[Bridge] Starting Motor feedback thread...")
    t_motor = threading.Thread(target=motor_thread, daemon=True)
    t_motor.start()
    
    while True:
        time.sleep(1)
