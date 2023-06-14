from sgp4.api import Satrec, days2mdhms, jday
from tabulate import tabulate
from datetime import datetime, timedelta
from pprint import pprint
from math import sqrt


starlink_tle_file = "starlink-tle.txt"
css_tle_file = "css-tle.txt"
dist_warning_threshold_in_km = 250


def tles2sats(filename):
    f = open(filename, 'r')
    l1, l2 = [], []
    i = 1
    for line in f:
        if i % 2 == 1:
            l1.append(line[:69])
        else:
            l2.append(line[:69])
        i = i + 1
    tles = []
    for i in range(len(l1)):
        # https://pypi.org/project/sgp4/
        tles.append(Satrec.twoline2rv(l1[i], l2[i]))
    return tles, datetime_from_sat(tles[0]), datetime_from_sat(tles[-1])
    
def datetime_from_sat(sat):
    month, day, hour, minute, sec = days2mdhms(sat.epochyr, sat.epochdays)
    return datetime(2000+sat.epochyr, month, day, hour, minute, int(sec))
    
def get_sat(sats, cur_idx, t):
    if cur_idx < len(sats) - 1 and datetime_from_sat(sats[cur_idx + 1]) <= t:
        cur_idx = cur_idx + 1
    return sats[cur_idx], cur_idx


starlink_sats, starlink_tle_start_time,  starlink_tle_end_time = tles2sats(starlink_tle_file)
css_sats, css_tle_start_time,  css_tle_end_time = tles2sats(css_tle_file)
start_time = min(starlink_tle_start_time, css_tle_start_time) - timedelta(seconds=1)
end_time = max(starlink_tle_end_time, css_tle_end_time) + timedelta(seconds=1)

print()
print("scan start time: ", start_time)
print("scan end time  : ", end_time)
print("dist warning km: ", dist_warning_threshold_in_km)
print()


delta = end_time - start_time
starlink_idx, css_idx = 0, 0
for m in range(round(delta.total_seconds()) + 1): ## checking distance at each second
    t = start_time + timedelta(minutes = m)
    starlink_sat, starlink_idx = get_sat(starlink_sats, starlink_idx, t)
    css_sat, css_idx = get_sat(css_sats, css_idx, t)
    jd, fr = jday(t.year, t.month, t.day, t.hour, t.minute, t.second)
    e, starlink_r, starlink_v = starlink_sat.sgp4(jd, fr)
    e, css_r, css_v = css_sat.sgp4(jd, fr)
    dist = round(sqrt((starlink_r[0] - css_r[0])**2 + (starlink_r[1] - css_r[1])**2 + (starlink_r[2] - css_r[2])**2), 1)
    if dist < dist_warning_threshold_in_km:
        print("timestmap: ", t, "\tdist: ", dist)