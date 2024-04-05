import time
def switchToWindow(
        window_id,
        POS_X,
        POS_Y
        ):
    subprocess.run(["xdotool", "windowactivate", str(window_id)], check=True)
    subprocess.run(["xdotool", "mousemove", str(POS_X), str(POS_Y)], check=True)
    time.sleep(0.05)
    subprocess.run(["xdotool", "mousedown", "1"], check=True)
    subprocess.run(["xdotool", "mouseup", "1"], check=True)

def focusOnNearestEnemy():
    time.sleep(0.2)
    subprocess.run(["xdotool", "key", "p"], check=True)

if __name__ == "__main__":
    import sys
    import os
    import subprocess
    # windows labels map
    ids = [
        "186646529",
        "167772161",
        "178257921",
        "195035137",
    ]
    clientRole = [
            "TANK",
            "MAGE",
            "DRUID",
            "PALLY"
    ]
    w = { k:v for k,v in zip(clientRole,ids)
    }
    w_rev = {v:k for k,v in w.items()}


    p = {
            "TANK": (480,250), # TANK
            "MAGE": (1400,250), # MAGE
            "DRUID": (750,950),   # BALANCE DRUID
            "PALLY": (1400,850),# PALLY
    }
    command= sys.argv[1]
    if command == 'FOCUS':
        focusOnNearestEnemy()
        exit()
        
    TOID=w[command]
    COMINGID=subprocess.check_output(["xdotool","getactivewindow"]).decode("utf-8").strip()
    print(COMINGID, '->', TOID)
    print(w_rev[COMINGID], '->', w_rev[TOID])
    if COMINGID == w['TANK'] and  TOID == w['MAGE']:
        switchToWindow(TOID, *p[w_rev[TOID]])
        # focusOnNearestEnemy()
    elif COMINGID == w['MAGE'] and  TOID == w['TANK']:
        switchToWindow(TOID, *p[w_rev[TOID]])
    else:
        switchToWindow(TOID, *p[w_rev[TOID]])
