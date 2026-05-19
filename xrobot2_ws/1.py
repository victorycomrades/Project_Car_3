import numpy as np
#arzd = [-73.32,3.68,37.58,-4.07,54.59,-133.97]  #我们组
#arzd = [-21.86,30.10-90,9.24+90,59.79,77.98-90,-21.71]
arzd = [ -73.32,90-3.68,-37.58,-4.07,-54.59,0]
arz = np.radians(arzd)
print(arz)
# ===================== 最新 6 轴 DH 参数（你给的） =====================

d   = [399.1, 0, 0, 351, 0, 82]
a   = [0, 350, 42, 0, 0, 0]
arxd = [ 90,0,90,-90,90,0]
arx = np.radians(arxd)
##arx = [0,0,0,0,0,0]
# ===================== 三角函数数组（6个） =====================
coz = [0]*6
siz = [0]*6
cox = [0]*6
six = [0]*6

for i in range(6):
    coz[i] = np.cos(arz[i])
    siz[i] = np.sin(arz[i])
    cox[i] = np.cos(arx[i])
    six[i] = np.sin(arx[i])

# ===================== 6个 DH 矩阵 =====================
H = [0]*6

for i in range(6):

    H[i] = np.array([[coz[i] ,-siz[i]*cox[i],siz[i]*six[i],  a[i]*coz[i]],
                     [siz[i] ,coz[i]*cox[i] ,-coz[i]*six[i], a[i]*siz[i]],
                     [0      ,six[i]        ,cox[i]        , d[i]       ],
                     [0      ,0             ,0             , 1          ]
                     ])

# ===================== 6个矩阵连乘 + 法兰 =====================
T = np.eye(4)
for i in range(6):
    T = np.dot(T , H[i])

# ===================== 输出 =====================
print("===== 最终 4x4 变换矩阵 =====")
print(T)

print("\n===== 末端坐标 X Y Z (mm) =====")
print(f"X = {T[0,3]:.2f}")
print(f"Y = {T[1,3]:.2f}")
print(f"Z = {T[2,3]:.2f}")
print("\n=====  误差 =====")
print(f"ΔX = {T[0,3] - 83.23:.2f} mm")
print(f"ΔY = {T[1,3] - (-294.27):.2f} mm")
print(f"ΔZ = {T[2,3] - 467.04:.2f} mm")