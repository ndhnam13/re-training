def transform(serial_char):
    if (serial_char - 48) & 0xFFFFFFFF <= 9:
        return (serial_char - 48) & 0xFFFFFFFF
    if (serial_char - 79) & 0xDF == 0:
        return 0
    if serial_char == ord('l'):
        return 1
    if (serial_char - 97) & 0xFFFFFFFF <= 0x19:
        return (serial_char - 87) & 0xFFFFFFFF
    if (serial_char - 65) & 0xFFFFFFFF <= 0x19:
        return (serial_char - 55) & 0xFFFFFFFF
    return 0

def find_valid_serial():    
    target = [0xED]
    print(f"Target: {[hex(x) for x in target]}")
    for i in range(32, 127):
        for j in range(32, 127):
            result = (transform(i) * 16 + transform(j)) & 0xFF
            if result in target:
                print(chr(i) + chr(j))

lookup_table = [969622712,
  594890599,
  1593930257,
  1052452058,
  890701766,
  1677293387,
  394424968,
  266815521,
  1532978959,
  1211194088,
  2019260265,
  729421127,
  953225874,
  1117854514,
  892543556,
  2000911200,
  514538256,
  1400963072,
  486675118,
  1862498216,
  1136668818,
  758909582,
  1653935295,
  821063674,
  888606944,
  687085563,
  890056597,
  1513495898,
  365692427,
  184357836,
  677395407,
  863045227,
  818746596,
  391985767,
  1842768403,
  758385145,
  1478392706,
  1985112985,
  1552765320,
  746944881,
  368385984,
  1758203153,
  1240817244,
  660489060,
  756944316,
  1290697955,
  844453952,
  288239112,
  1769473626,
  1922176006,
  826636519,
  391520695,
  1081548223,
  1069693142,
  1244729994,
  766313326,
  1101031894,
  624951698,
  14501479,
  1794907983,
  1460682958,
  1660839647,
  1104890686,
  897721119,
  1442187162,
  480708164,
  454443986,
  1064446153,
  1595150448,
  1041527979,
  1145775470,
  1399869657,
  255985995,
  802693350,
  2005610078,
  1897360642,
  2146073193,
  1538606632,
  431647857,
  964049561,
  395138253,
  19164808,
  856904574,
  730737943,
  708645054,
  1506870658,
  933323739,
  819349658,
  1780571206,
  236747382,
  533160167,
  2042104933,
  670325172,
  2040165158,
  1354372994,
  705785180,
  1669754395,
  1066536508,
  1426207888,
  1437950089,
  741941201,
  796931522,
  1694313338,
  1290302874,
  1367672048,
  2039808424,
  1062939821,
  954597728,
  1668694488,
  859122242,
  1369582617,
  140269649,
  53024683,
  729221831,
  816609203,
  736893191,
  55706320,
  262747091,
  1629838835,
  581764799,
  1488480625,
  1607077349,
  1879925846,
  1453945819,
  1521965565,
  856558562,
  1530662365,
  1230847072,
  1404918182,
  1281256849,
  1238970765,
  272453753,
  1640907491,
  2127893021,
  350314733,
  556617458,
  654390256,
  1648581270,
  531062411,
  1862873022,
  1241517385,
  1471028336,
  5121143,
  1444839026,
  1183580211,
  1573659650,
  2018540230,
  1487873223,
  234237236,
  898254600,
  1023090193,
  728843548,
  2007454357,
  1451820833,
  267351539,
  302982385,
  26807015,
  865879122,
  664886158,
  195503981,
  1625037691,
  1330347906,
  1742434311,
  1330272217,
  1645368040,
  542321916,
  1782121222,
  411042851,
  435386250,
  1176704752,
  1454246199,
  1136813916,
  1707755005,
  224415730,
  201138891,
  989750331,
  1006010278,
  1147286905,
  406860280,
  840388503,
  1282017578,
  1605698145,
  23396724,
  862145265,
  1898780916,
  1855549801,
  1571519230,
  2083204840,
  1859876276,
  1602449334,
  1009413590,
  690816450,
  86131931,
  345661263,
  1565025600,
  857544170,
  1329948960,
  1211787679,
  994381573,
  991984748,
  1956475134,
  1098146294,
  1655714289,
  659576699,
  689116467,
  1485584392,
  451884118,
  255590636,
  2108114754,
  1266252396,
  1589326471,
  2019907768,
  15552498,
  1651075358,
  614606175,
  1656823678,
  797605325,
  1681594366,
  2005080248,
  624648446,
  884695971,
  1526931791,
  1595240948,
  439447199,
  2060396292,
  680093752,
  409028215,
  469068267,
  195583689,
  1791650630,
  507724330,
  1364025102,
  1094582668,
  813049577,
  32316922,
  1240756058,
  1176200235,
  2104494066,
  325396055,
  1796606917,
  1709197385,
  525495836,
  1510101430,
  735526761,
  767523533,
  1374043776,
  1559389967,
  567085571,
  1560216161,
  867042846,
  1001796703,
  1568754293,
  628841972,
  173812827,
  379868455,
  384973125,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0]

def name_hashing(name_data, a2, a3, a4):
    name_data_len = len(name_data)
    name_data = name_data.upper()
    # print(name_data)

    v5 = 0
    v9 = 0
    v10 = (15 * a4) & 0xFF
    v11 = 0
    v12 = (17 * a3) & 0xFF
    for char in name_data:
        v13 = ord(char)
        v14 = v13
        v15 = lookup_table[v12] & 0xFFFFFFFF
        v16 = lookup_table[v10] & 0xFFFFFFFF
        v17 = (v5 + lookup_table[v13]) & 0xFFFFFFFF
        if (a2):
            v18 = (lookup_table[(v14 + 13)&0xFF] ^ v17) & 0xFFFFFFFF
            v19 = (v14 + 47) & 0xFF
            v20 = v9
        else:
            v18 = (lookup_table[(v14 + 63)&0xFF] ^ v17) & 0xFFFFFFFF
            v19 = (v14 + 23) & 0xFF
            v20 = v11
        v12 = (v12 + 9) & 0xFF
        v10 = (v10 + 13) & 0xFF
        v9 = (v9 + 19) & 0xFF
        v11 = (v11 + 7) & 0xFF
        v5 = (v16 + v15 + lookup_table[v20] + lookup_table[v19] * v18) & 0xFFFFFFFF
    
    return v5

def gen_first_3_bytes(name_hash):
    v2 = 17

    tmp = v2 ^ 0xFFE53167
    tmp = (tmp + 180597) & 0xFFFFFFFF
    
    a1 = (tmp ^ name_hash ^ 0x22C078) & 0xFFFFFFFF
    
    byte0 = a1 & 0xFF
    byte1 = (a1 >> 8) & 0xFF
    byte2 = (a1 >> 16) & 0xFF
    
    return byte0, byte1, byte2, a1

def name_hash_check(a1, name_hash):
    v2 = (((name_hash ^ a1 ^ 0x22C078) - 180597) ^ 0xFFE53167) & 0xFFFFFF
    if ( v2 == 17 * (v2 // 17) ):
        return v2 // 17

    return 0

def case_0xFC(name_hash):
    byte0, byte1, byte2, a1 = gen_first_3_bytes(name_hash)
    result = name_hash_check(a1, name_hash)
    if (result):
        print(f"Target a1: {hex(a1)}")
        print(f"Byte 0 (decoded_bytes): {hex(byte0)}")
        print(f"Byte 1 (a2_1): {hex(byte1)}")
        print(f"Byte 2 (a2_2): {hex(byte2)}")

def gen_serial_0xAC(name_hash, v23):
    # First check
    v1 = 11
    tmp = v1 ^ 0x3421
    tmp = (tmp - 19760) & 0xFFFFFFFF
    a1 = (tmp ^ 0x7892) & 0xFFFFFFFF
    b5_xor_b2 = a1 & 0xFF
    b7_xor_b1 = (a1>>8) & 0xFF
    print(f"5 xor 2: {hex(b5_xor_b2)}\n7 xor 1: {hex(b7_xor_b1)}")
    # 2nd check
    if ( ((((a1^0x7892)+19760) & 0xFFFFFFFF)^0x3421) // 11 <= 5000 ):
        v2 = v23 * 17 # v2 % 17 and v2 // 17 > 20300
        tmp = v2 ^ 0xFFE53167
        tmp = (tmp + 180597) & 0xFFFFFFFF
        a2 = (tmp ^ 0x5B8C27 ^ 0x22C078) & 0xFFFFFFFF
        b6_xor_b0 = a2 & 0xFF
        b8_xor_b4 = (a2 >> 8) & 0xFF
        b5_xor_b9 = (a2 >> 16) & 0xFF
        print(f"6 xor 0: {hex(b6_xor_b0)}\n8 xor 4: {hex(b8_xor_b4)}\n5 xor 9: {hex(b5_xor_b9)}")
        # Generate serial
        decoded_bytes = [0] * 10
        decoded_bytes[3] = 0xAC
        decoded_bytes[4] = name_hash & 0xFF
        decoded_bytes[5] = (name_hash >> 8) & 0xFF
        decoded_bytes[6] = (name_hash >> 16) & 0xFF
        decoded_bytes[7] = (name_hash >> 24) & 0xFF
        decoded_bytes[2] = decoded_bytes[5] ^ b5_xor_b2
        decoded_bytes[1] = decoded_bytes[7] ^ b7_xor_b1     
        decoded_bytes[0] = decoded_bytes[6] ^ b6_xor_b0
        decoded_bytes[8] = decoded_bytes[4] ^ b8_xor_b4
        decoded_bytes[9] = decoded_bytes[5] ^ b5_xor_b9
        hex_bytes = [f"{x:02X}" for x in decoded_bytes]
        groups = []
        for i in range(0, 10, 2):
            groups.append(f"{hex_bytes[i]}{hex_bytes[i+1]}")
        serial_formatted = "-".join(groups)        
        print(f"Serial: {serial_formatted}")


def main():
    # find_valid_serial()
    signature_byte = 0xAC
    if (signature_byte == 0xAC):
        v4 = 1
        v23 = 200000 # Must be > 20300 and v23 * 86400 should be larger than current date
    if (signature_byte == 0xFC):
        v4 = 0
        v23 = 255
    
    name = "hoangnam"
    name_hash = name_hashing(name, v4, v23, 1)  # 0xFC: 0x8DD84B9B
                                                # 0xAC: 0xA5BB9A3C
    print(f"Hash for {name}: {hex(name_hash)}")

    if (signature_byte == 0xAC):
        gen_serial_0xAC(name_hash, v23)
    if (signature_byte == 0xFC):
        case_0xFC(name_hash)


main()