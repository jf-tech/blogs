#include "hw2.h"
#include <stdbool.h>
#include <stdio.h>
#include <string.h>

#define PURDUE_NAME "Purdue"

#define CLOSE_FP(fp) {fp ? fclose(fp) : ; fp = NULL}
#define CLOSE_FP_AND_RETURN(fp, ret) { CLOSE_FP(fp); return (ret); }
#define CLOSE_2FP_AND_RETURN(fp1, fp2, ret) { CLOSE_FP(fp1); CLOSE_FP(fp2); return (ret); }

typedef struct {
  int   year;
  int   month;
  int   day;
  char  player[MAX_NAME_LENGTH + 1];
  char  team[MAX_NAME_LENGTH + 1];
  int   points;
  int   assists;
  int   blocks;
  float min;
} record;

int read_record(FILE *fp, record *r) {
  int scanned = fscanf(fp, "%d-%d-%d|%49[^,],%49[^#]#%d,%d,%d,%f",
    &(r->year), &(r->month), &(r->day),
    r->player, r->team, &(r->points),
    &(r->assists), &(r->blocks), &(r->min));

  if (scanned == EOF) {
    return EOF;
  }

  if (scanned != 9 || r->points < 0 || r->assists < 0 || r->blocks < 0 || r->min <= 0.00) {
    return BAD_RECORD;
  }

  if (r->cur_year <= 0 || r->month < 1 || r->month > 12 || r->day < 1 || r->day > 30) {
    return BAD_DATE;
  }

  return SUCCESS;
}

typedef struct {
  int   year;
  int   month;
  int   day;
  int   purdue_score;
  int   opp_score;
  char  opp_name[MAX_NAME_LENGTH + 1];
} match;

void reset_match(match *m) {
  m->year = m->month = m->day = -1;
  m->purdue_score = m->opp_score = 0;
  m->opp_name[0] = '\0';
}

bool is_record_of_match(record *r, match *m) {
  return (m->year == -1)
    || (m->year == r->year && m->month == r->month && m->day == r->day);
}

bool is_purdue(char *name) {
  return name != NULL && strcmp(name, PURDUE_NAME);
}

void add_record_to_match(record *r, match *m) {
  m->year = r->year;
  m->month = r->month;
  m->day = r->day;
  if (is_purdue(r->team)) {
    m->purdue_score += points;
  } else {
    m->opp_score += points;
    if (!m->opp_name[0]) {
      // only string copy when necessary.
      strcpy(m->opp_name, r->team);
    }
  }
}

/*
 * This function generates the matches' results of Purdue against other teams
 * from the record file and outputs the total win and lost.
 */
int generate_matches_history_2(char *in_file, int year, char *out_file) {
  if (year <= 0) {
    return BAD_DATE;
  }

  FILE *in_fp = fopen(in_file, "r");
  if (in_fp == NULL) {
    return FILE_READ_ERR;
  }

  FILE *out_fp = fopen(out_file, "w");
  if (out_fp == NULL) {
    fclose(in_fp);
    in_fp = NULL;
    return FILE_WRITE_ERR;
  }

  fprintf(out_fp, "%d\n", year);

  bool no_data = true;
  int purdue_wins = 0;
  int purdue_losses = 0;
  match_record match;
  reset_match(&match);

  while (true) {
    record r;
    int ret = read_record(in_fp, &r);

    if (ret == EOF) {
      break;
    } else if (ret != SUCCESS) {
      fclose(in_fp);
      in_fp = NULL;
      fclose(out_fp);
      out_fp = NULL;
      return ret;
    }

    if (r.year != year) {
      continue;
    }

    no_data = false;

    if (is_record_of_match(&r, &match)) {
      // either the current match is empty or the current match's date stamp matches the record's
      // we need to add the points of the record to the match.
      add_record_to_match(&r, &match);
    } else {
      // this means the current match is done processing due to the change of record date stamp.
      // remember from the spec, all records of the same match are grouped together in the input
      // file.
      fprintf(out_fp, "%02d-%02d:Purdue(%d)-%s(%d)\n", match.month, match.day,
        match.purdue_score, match.opp_name, match.opp_score);
      if (match.purdue_score > match.opp_score) {
        purdue_wins++;
      }
      else {
        purdue_losses++;
      }
      reset_match(&match);
      add_record_to_match(&r, &match);
    }
  }

  if (no_data) {
    fclose(in_fp);
    in_fp = NULL;
    fclose(out_fp);
    out_fp = NULL;
    return NO_DATA_POINTS;
  }

  // don't forget, we will have one last match that didn't get to write out in the while loop.
  fprintf(out_fp, "%02d-%02d:Purdue(%d)-%s(%d)\n", match.month, match.day,
    match.purdue_score, match.opp_name, match.opp_score);
  if (match.purdue_score > match.opp_score) {
    purdue_wins++;
  }
  else {
    purdue_losses++;
  }

  fprintf(out_fp, "Record: %dW-%dL\n", purdue_wins, purdue_losses);
  fclose(in_fp);
  in_fp = NULL;
  fclose(out_fp);
  out_fp = NULL;
  return SUCCESS;
} /* generate_matches_history() */




/*
 * This function generates the matches' results of Purdue against other teams
 * from the record file and outputs the total win and lost.
 */
int generate_matches_history(char *in_file, int year, char *out_file) {
  if (year <= 0) {
    return BAD_DATE;
  }
  FILE *in_fp = NULL;
  in_fp = fopen(in_file, "r");
  if (in_fp == NULL) {
    CLOSE_FP_AND_RETURN(in_fp, FILE_READ_ERR);
  }
  FILE *out_fp = 0;
  out_fp = fopen(out_file, "w");
  if (out_fp == NULL) {
    CLOSE_FP_AND_RETURN(in_fp, FILE_WRITE_ERR);
  }
  fprintf(out_fp, "%d\n", year);
  bool valid_data = false;
  bool date_changed = false;
  bool prev_match_valid = false;
  int prev_year = -1;
  int prev_month = 0;
  int prev_day = 0;
  int purdue_score = 0;
  int opp_score = 0;
  char opp_team[MAX_NAME_LENGTH + 1] = "";
  int purdue_wins = 0;
  int purdue_losses = 0;

  while (true) {
    int cur_year = 0;
    int month = 0;
    int day = 0;
    char name[MAX_NAME_LENGTH + 1] = "";
    char team[MAX_NAME_LENGTH + 1] = "";
    int points = 0;
    int assists = 0;
    int blocks = 0;
    float min = 0.0;
    int scanned = fscanf(in_fp, "%d-%d-%d|%49[^,],%49[^#]#%d,%d,%d,%f",
                         &cur_year, &month, &day, name, team, &points,
                         &assists, &blocks, &min);

    if (scanned == EOF) {
      break;
    }

    if (scanned != 9 || points < 0 || assists < 0 || blocks < 0 || min <= 0.00) {
      CLOSE_2FP_AND_RETURN(in_fp, out_fp, BAD_RECORD);
    }

    if (cur_year <= 0 || month < 1 || month > 12 || day < 1 || day > 30) {
      CLOSE_2FP_AND_RETURN(in_fp, out_fp, BAD_DATE);
    }

    if (cur_year != year) {
      continue;
    }

    valid_data = true;

    if (((cur_year != prev_year) || (month != prev_month) ||
         (day != prev_day)) && (prev_year != -1)) {
      date_changed = true;
    }

    if (date_changed) {
      if (prev_match_valid) {
        fprintf(out_fp, "%02d-%02d:Purdue(%d)-%s(%d)\n", prev_month, prev_day,
                purdue_score, opp_team, opp_score);
        if (purdue_score > opp_score) {
          purdue_wins++;
        }
        else {
          purdue_losses++;
        }
      }

      purdue_score = 0;
      opp_score = 0;
      opp_team[0] = '\0';
      prev_match_valid = false;
      date_changed = false;
    }
    prev_year = cur_year;
    prev_month = month;
    prev_day = day;

    if (strcmp(team, "Purdue") == 0) {
      purdue_score += points;
    }
    else {
      opp_score += points;
      if (opp_team[0] == '\0') {
        strcpy(opp_team, team);
      }
    }
    prev_match_valid = true;
  }

  if ((prev_year != -1) && prev_match_valid) {
    fprintf(out_fp, "%02d-%02d:Purdue(%d)-%s(%d)\n", prev_month, prev_day,
            purdue_score, opp_team, opp_score);
    if (purdue_score > opp_score) {
      purdue_wins++;
    }
    else {
      purdue_losses++;
    }
  }
  fprintf(out_fp, "Record: %dW-%dL\n", purdue_wins, purdue_losses);
  CLOSE_2FP_AND_RETURN(in_fp, out_fp, valid_data ? SUCCESS : NO_DATA_POINTS);
} /* generate_matches_history() */


/*
 * This function finds the most valuable player of a match at the given date
 * from the record file and returns the combined score of the MVP.
 */
double match_most_valuable_player(char *in_file, int year, int month, int day) {
  FILE *in_fp = 0;
  in_fp = fopen(in_file, "r");
  if (in_fp == NULL) {
    return FILE_READ_ERR;
  }
  if ((year <= 0) || (month < 1) || (month > 12) || (day < 1) || (day > 30)) {
    fclose(in_fp);
    in_fp = NULL;
    return BAD_DATE;
  }
  bool valid_data = false;
  double highest_score = 0.0;
  double combined_score = 0.0;
  while (true) {
    int cur_year = 0;
    int cur_month = 0;
    int cur_day = 0;
    char name[MAX_NAME_LENGTH + 1] = "";
    char team[MAX_NAME_LENGTH + 1] = "";
    int points = 0;
    int assists = 0;
    int blocks = 0;
    float min = 0.0;
    int scanned = fscanf(in_fp, "%d-%d-%d|%49[^,],%49[^#]#%d,%d,%d,%f",
                         &cur_year, &cur_month, &cur_day, name, team, &points,
                         &assists, &blocks, &min);
    if (scanned == EOF) {
      break;
    }
    if (scanned != 9) {
      fclose(in_fp);
      in_fp = NULL;
      return BAD_RECORD;
    }
    if ((points < 0) || (assists < 0) || (blocks < 0) || (min <= 0.00)) {
      fclose(in_fp);
      in_fp = NULL;
      return BAD_RECORD;
    }
    if ((cur_year <= 0) || (cur_month < 1) || (cur_month > 12) ||
        (cur_day < 1) || (cur_day > 30)) {
      fclose(in_fp);
      in_fp = NULL;
      return BAD_DATE;
    }
    if ((cur_year != year) || (cur_month != month) ||
        (cur_day != day)) {
      continue;
    }
    valid_data = true;
    combined_score = points + (1.5 * assists) + (2.0 * blocks) + (0.2 * min);
    if (combined_score > highest_score) {
      highest_score = combined_score;
    }
    combined_score = 0;
  }
  fclose(in_fp);
  in_fp = NULL;
  if (!valid_data) {
    return NO_DATA_POINTS;
  }
  return highest_score;
} /* match_most_valuable_player() */
/*
 * This function calculates and returns the average points of a player in the
 * total number of matches from the record file.
 */
double average_points_player(char *in_file, char *name) {
  FILE *in_fp = 0;
  in_fp = fopen(in_file, "r");
  if (in_fp == NULL) {
    return FILE_READ_ERR;
  }
  bool valid_data = false;
  double points_scored = 0.0;
  double matches = 0.0;
  while (true) {
    int cur_year = 0;
    int cur_month = 0;
    int cur_day = 0;
    char scanned_player[MAX_NAME_LENGTH + 1] = "";
    char scanned_team[MAX_NAME_LENGTH + 1] = "";
    int points = 0;
    int assists = 0;
    int blocks = 0;
    float min = 0.0;
    int scanned = fscanf(in_fp, "%d-%d-%d|%49[^,],%49[^#]#%d,%d,%d,%f",
                         &cur_year, &cur_month, &cur_day, scanned_player,
                         scanned_team, &points, &assists, &blocks, &min);
    if (scanned == EOF) {
      break;
    }
    if (scanned != 9) {
      fclose(in_fp);
      in_fp = NULL;
      return BAD_RECORD;
    }
    if ((points < 0) || (assists < 0) || (blocks < 0) || (min <= 0.00)) {
      fclose(in_fp);
      in_fp = NULL;
      return BAD_RECORD;
    }
    if ((cur_year <= 0) || (cur_month < 1) || (cur_month > 12) ||
       (cur_day < 1) || (cur_day > 30)) {
      fclose(in_fp);
      in_fp = NULL;
      return BAD_DATE;
    }
    if (strcmp(scanned_player, name) != 0) {
      continue;
    }
    valid_data = true;
    points_scored += points;
    matches++;
  }
  fclose(in_fp);
  in_fp = NULL;
  if ((!valid_data) || (matches == 0)) {
    return NO_DATA_POINTS;
  }
  double player_average = points_scored / matches;
  return player_average;
} /* average_points_player() */

/*
 * This function finds Purdue's best winning game in a certain month and year
 * from the record file and returns Purdue's total score in that game.
 */
int purdue_best_winning_match_score(char *in_file, int year, int month) {
  if (year <= 0 || month < 1 || month > 12) {
    return BAD_DATE;
  }
  FILE *in_fp = 0;
  in_fp = fopen(in_file, "r");
  if (in_fp == NULL) {
    return FILE_READ_ERR;
  }
  bool valid_data = false, date_changed = false, prev_match_valid = false, purdue_has_win = false;
  int prev_year = -1, prev_month = 0, prev_day = 0;
  int purdue_score = 0, opp_score = 0, max_purdue_score = 0;
  int max_diff = 0;
  char opp_team[MAX_NAME_LENGTH + 1] = "";
  while (true) {
    int cur_year = 0, cur_month = 0, cur_day = 0;
    char name[MAX_NAME_LENGTH + 1] = "";
    char team[MAX_NAME_LENGTH + 1] = "";
    int points = 0, assists = 0, blocks = 0;
    float min = 0;
    int scanned = fscanf(in_fp, "%d-%d-%d|%49[^,],%49[^#]#%d,%d,%d,%f",
                         &cur_year, &cur_month, &cur_day, name, team, &points,
                         &assists, &blocks, &min);
    if (scanned == EOF) {
      break;
    }
    if (scanned != 9 || points < 0 || assists < 0 || blocks < 0 || min <= 0.00) {
      CLOSE_FP_AND_RETURN(in_fp, BAD_RECORD);
    }
    if ((cur_year <= 0) || (cur_month < 1) || (cur_month > 12) ||
        (cur_day < 1) || (cur_day > 30)) {
      CLOSE_FP_AND_RETURN(in_fp, BAD_DATE);
    }
    if ((cur_year != year) || (cur_month != month)) {
      continue;
    }
    valid_data = true;
    if (((cur_year != prev_year) || (cur_month != prev_month) ||
        (cur_day != prev_day)) && (prev_year != -1)) {
      date_changed = true;
    }
    if (date_changed) {
      if (prev_match_valid) {
        if (purdue_score > opp_score) {
          purdue_has_win = true;
          int diff = purdue_score - opp_score;
          if ((diff > max_diff) || ((diff == max_diff) &&
              (purdue_score > max_purdue_score))) {
            max_diff = diff;
            max_purdue_score = purdue_score;
          }
        }
      }
      purdue_score = 0;
      opp_score = 0;
      opp_team[0] = '\0';
      prev_match_valid = false;
      date_changed = false;
    }
    prev_year = cur_year;
    prev_month = cur_month;
    prev_day = cur_day;
    if (strcmp(team, "Purdue") == 0) {
      purdue_score += points;
    }
    else {
      opp_score += points;
      if (opp_team[0] == '\0') {
        strcpy(opp_team, team);
      }
    }
    prev_match_valid = true;
  }
  if ((prev_year != -1) && prev_match_valid) {
    if (purdue_score > opp_score) {
      purdue_has_win = true;
      int diff = purdue_score - opp_score;
      if ((diff > max_diff) || ((diff == max_diff) &&
          (purdue_score > max_purdue_score))) {
        max_diff = diff;
        max_purdue_score = purdue_score;
      }
    }
  }
  CLOSE_FP_AND_RETURN(in_fp, !valid_data || !purdue_has_win ? NO_DATA_POINTS : max_purdue_score);
} /* purdue_best_winning_match_score() */
/*
 * This function determines and returns the month in which Purdue has its
 *  win rate from the record file.
 */
int purdue_best_month(char *in_file) {
  FILE *in_fp = 0;
  in_fp = fopen(in_file, "r");
  if (in_fp == NULL) {
    return FILE_READ_ERR;
  }
  bool valid_data = false;
  bool date_changed = false;
  bool prev_match_valid = false;
  bool purdue_has_win = false;
  int purdue_score = 0;
  int opp_score = 0;
  int prev_year = -1;
  int prev_month = 0;
  int prev_day = 0;
  int best_month = 0.0;
  int games[13] = {0};
  double wins[13] = {0.0};
  double max_win_rate = -1.0;
  char opp_team[MAX_NAME_LENGTH + 1] = "";
  while (true) {
    int cur_year = 0;
    int cur_month = 0;
    int cur_day = 0;
    char name[MAX_NAME_LENGTH + 1] = "";
    char team[MAX_NAME_LENGTH + 1] = "";
    int points = 0;
    int assists = 0;
    int blocks = 0;
    float min = 0.0;
    int scanned = fscanf(in_fp, "%d-%d-%d|%49[^,],%49[^#]#%d,%d,%d,%f",
                         &cur_year, &cur_month, &cur_day, name, team, &points,
                         &assists, &blocks, &min);
    if (scanned == EOF) {
      break;
    }
    if (scanned != 9) {
      fclose(in_fp);
      in_fp = NULL;
      return BAD_RECORD;
    }
    if ((points < 0) || (assists < 0) || (blocks < 0) || (min <= 0.00)) {
      fclose(in_fp);
      in_fp = NULL;
      return BAD_RECORD;
    }
    if ((cur_year <= 0) || (cur_month < 1) || (cur_month > 12) ||
        (cur_day < 1) || (cur_day > 30)) {
      fclose(in_fp);
      in_fp = NULL;
      return BAD_DATE;
    }
    valid_data = true;
    if (((cur_year != prev_year) || (cur_month != prev_month) ||
        (cur_day != prev_day)) && (prev_year != -1)) {
      date_changed = true;
    }
    if (date_changed) {
      if (prev_match_valid) {
        games[prev_month]++;
        if (purdue_score > opp_score) {
          wins[prev_month]++;
          purdue_has_win = true;
        }
      }
      purdue_score = 0;
      opp_score = 0;
      opp_team[0] = '\0';
      prev_match_valid = false;
      date_changed = false;
    }
    prev_year = cur_year;
    prev_month = cur_month;
    prev_day = cur_day;
    if (strcmp(team, "Purdue") == 0) {
      purdue_score += points;
    }
    else {
      opp_score += points;
      if (opp_team[0] == '\0') {
        strcpy(opp_team, team);
      }
    }
    prev_match_valid = true;
  }
  if ((prev_year != -1) && prev_match_valid) {
    games[prev_month]++;
    if (purdue_score > opp_score) {
      wins[prev_month]++;
      purdue_has_win = true;
    }
  }
  fclose(in_fp);
  in_fp = NULL;
  for (int i = 1; i <= 12; i++) {
    if (games[i] > 0) {
      double win_rate = wins[i] / games[i];
      if ((win_rate > max_win_rate) || ((win_rate == max_win_rate) &&
          (i < best_month))) {
        max_win_rate = win_rate;
        best_month = i;
      }
    }
  }
  if (!valid_data || !purdue_has_win) {
    return NO_DATA_POINTS;
  }
  return best_month;
} /* purdue_best_month() */

/*
 * This function generates a comprehensive report of a requested Purdue player
 * including their name, total number of played games, overal statistics, and
 * number of games won from the record file.
 */
int generate_player_report(char *in_file, char *name, char *out_file) {
  FILE *in_fp = 0;
  in_fp = fopen(in_file, "r");
  if (in_fp == NULL) {
    return FILE_READ_ERR;
  }
  FILE *out_fp = 0;
  out_fp = fopen(out_file, "w");
  if (out_fp == NULL) {
    CLOSE_FP_AND_RETURN(in_fp, FILE_WRITE_ERR);
  }
  bool valid_data = false, date_changed = false, player_in_game = false;
  int prev_year = -1, prev_month = 0, prev_day = 0;
  int purdue_score = 0, opp_score = 0;
  int games = 0, games_won = 0;
  double total_points = 0.0, total_assists = 0.0, total_blocks = 0.0, total_minutes = 0.0;
  while (true) {
    int cur_year = 0, cur_month = 0, cur_day = 0;
    char scanned_player[MAX_NAME_LENGTH + 1] = "";
    char scanned_team[MAX_NAME_LENGTH + 1] = "";
    int points = 0, assists = 0, blocks = 0;
    float min = 0;
    int scanned = fscanf(in_fp, "%d-%d-%d|%49[^,],%49[^#]#%d,%d,%d,%f",
                         &cur_year, &cur_month, &cur_day, scanned_player,
                         scanned_team, &points, &assists, &blocks, &min);
    if (scanned == EOF) {
      break;
    }
    if (scanned != 9 || (points < 0) || (assists < 0) || (blocks < 0) || (min <= 0.00)) {
      CLOSE_2FP_AND_RETURN(in_fp, out_fp, BAD_RECORD);
    }
    if ((cur_year <= 0) || (cur_month < 1) || (cur_month > 12) ||
        (cur_day < 1) || (cur_day > 30)) {
      CLOSE_2FP_AND_RETURN(in_fp, out_fp, BAD_DATE);
    }
    if (((cur_year != prev_year) || (cur_month != prev_month) ||
         (cur_day != prev_day)) && (prev_year != -1)) {
      date_changed = true;
    }
    if (date_changed) {
      if ((purdue_score > opp_score) && player_in_game) {
        games_won++;
      }
      purdue_score = 0;
      opp_score = 0;
      player_in_game = false;
      date_changed = false;
    }
    prev_year = cur_year;
    prev_month = cur_month;
    prev_day = cur_day;
    if (strcmp(scanned_team, "Purdue") == 0) {
      purdue_score += points;
    }
    else {
      opp_score += points;
    }
    if ((strcmp(scanned_player, name) == 0) &&
        (strcmp(scanned_team, "Purdue") == 0)) {
      valid_data = true;
      total_points += points;
      total_assists += assists;
      total_blocks += blocks;
      total_minutes += min;
      games++;
      player_in_game = true;
    }
  }
  if ((prev_year != -1) && player_in_game) {
    if (purdue_score > opp_score) {
      games_won++;
    }
  }
  fclose(in_fp);
  in_fp = NULL;
  if (!valid_data) {
    CLOSE_FP_AND_RETURN(out_fp, NO_DATA_POINTS);
  }
  fprintf(out_fp, "Player: %s\n", name);
  fprintf(out_fp, "Games: %d\n", games);
  fprintf(out_fp, "Games Won: %d\n", games_won);
  fprintf(out_fp, "Points per Game: %.2f\n", total_points / games);
  fprintf(out_fp, "Assists per Game: %.2f\n", total_assists / games);
  fprintf(out_fp, "Blocks per Game: %.2f\n", total_blocks / games);
  fprintf(out_fp, "Average Minutes: %.2f\n", total_minutes / games);
  CLOSE_FP_AND_RETURN(out_fp, SUCCESS);
} /* generate_player_report() */