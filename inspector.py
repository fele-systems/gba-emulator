def print_all_bits(number):
    asstr = "{0:032b}".format(number)
    pt1 = asstr[0:8]
    pt2 = asstr[8:16]
    pt3 = asstr[16:24]
    pt4 = asstr[24:]

    print("   {}      {}      {}      {}   ".format(pt1, pt2, pt3, pt4))
    print(" |__________|  |__________|  |__________|  |__________|")
    print(" 31        24  23        16  15         8  7          0")


