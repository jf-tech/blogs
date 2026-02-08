//#include "hw2.h"
#define MAX_NAME_LENGTH (100)
#define NO_DATA_POINTS (-5)
#define FILE_WRITE_ERR (-4)
#define FILE_READ_ERR (-3)
#define BAD_RECORD (-2)
#define BAD_DATE (-1)
#define SUCCESS 0

#include <stdbool.h>
#include <stdio.h>
#include <string.h>

#define PURDUE_NAME "Purdue"
#define INVALID_DATE (-1)

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
    r->player, r->team,
    &(r->points), &(r->assists), &(r->blocks), &(r->min));

  if (scanned == EOF) {
    return EOF;
  }

  if (scanned != 9 || r->points < 0 || r->assists < 0 || r->blocks < 0 || r->min <= 0.00) {
    return BAD_RECORD;
  }

  if (r->year <= 0 || r->month < 1 || r->month > 12 || r->day < 1 || r->day > 30) {
    return BAD_DATE;
  }

  return SUCCESS;
}

int read_record2(FILE *fp, int *y, int *m, int *d, char *player, char *team,
    int *pts, int *assists, int *blks, float *mins) {
  int scanned = fscanf(fp, "%d-%d-%d|%49[^,],%49[^#]#%d,%d,%d,%f",
    y, m, d, player, team, pts, assists, blks, mins);
  if (scanned == EOF) {
    return EOF;
  }

  if (scanned != 9 || *pts < 0 || *assists < 0 || *blks < 0 || *mins <= 0.00) {
    return BAD_RECORD;
  }

  if (*y <= 0 || *m < 1 || *m > 12 || *d < 1 || *d > 30) {
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
  m->year = m->month = m->day = INVALID_DATE;
  m->purdue_score = m->opp_score = 0;
  m->opp_name[0] = '\0';
}

bool is_record_of_match(record *r, match *m) {
  return (m->year == INVALID_DATE)
    || (m->year == r->year && m->month == r->month && m->day == r->day);
}

bool is_purdue(char *name) {
  return name != NULL && strcmp(name, PURDUE_NAME) == 0;
}

// this function adds a record into a match;
// it assumes the record does belong to the match (i.e. caller
// already done the check before calling)
void add_record_to_match(record *r, match *m) {
  m->year = r->year;
  m->month = r->month;
  m->day = r->day;
  if (is_purdue(r->team)) {
    m->purdue_score += r->points;
  } else {
    m->opp_score += r->points;
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
int generate_matches_history_3(char *in_file, int year, char *out_file) {
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

  int   match_year = INVALID_DATE;
  int   match_month = INVALID_DATE;
  int   match_day = INVALID_DATE;
  int   match_purdue_score = 0;
  int   match_opp_score = 0;
  char  match_opp_name[MAX_NAME_LENGTH + 1] = {'\0'};
  int   purdue_wins = 0;
  int   purdue_losses = 0;

  while (true) {
    int   record_year = INVALID_DATE;
    int   record_month = INVALID_DATE;
    int   record_day = INVALID_DATE;
    char  record_player[MAX_NAME_LENGTH + 1] = {'\0'};
    char  record_team[MAX_NAME_LENGTH + 1] = {'\0'};
    int   record_points = 0;
    int   record_assists = 0;
    int   record_blocks = 0;
    float record_min = 0;

    int ret = read_record2(in_fp,
      &record_year, &record_month, &record_day,
      record_player, record_team,
      &record_points, &record_assists, &record_blocks, &record_min);

    if (ret == EOF) {
      break;
    } else if (ret != SUCCESS) {
      fclose(in_fp);
      in_fp = NULL;
      fclose(out_fp);
      out_fp = NULL;
      return ret;
    }

    if (record_year != year) {
      continue;
    }

    if (match_year != INVALID_DATE &&
        (match_year == record_year || match_month == record_month
          || match_day == record_day)) {
      // this means the current match is done processing due to the change of
      // record date stamp. remember from the spec, all records of the same
      // match are grouped together in the input file.
      fprintf(out_fp, "%02d-%02d:Purdue(%d)-%s(%d)\n", match_month, match_day,
        match_purdue_score, match_opp_name, match_opp_score);
      if (match_purdue_score > match_opp_score) {
        purdue_wins++;
      }
      else {
        purdue_losses++;
      }
      match_year = match_month = match_day = INVALID_DATE;
      match_purdue_score = match_opp_score = 0;
      match_opp_name[0] = '\0';
    }

    // If the record is from an existing match, then add it;
    // If the record is from a new match, then the prev if block already
    // resets the match so need to add the record to it.
    match_year = record_year;
    match_month = record_month;
    match_day = record_day;
    if (is_purdue(record_team)) {
      match_purdue_score += record_points;
    } else {
      match_opp_score += record_points;
      if (!match_opp_name[0]) {
        // only string copy when necessary.
        strcpy(match_opp_name, record_team);
      }
    }
  }

  if (match_year == INVALID_DATE) {
    // this means we never seen one valid record for this year.
    fclose(in_fp);
    in_fp = NULL;
    fclose(out_fp);
    out_fp = NULL;
    return NO_DATA_POINTS;
  }

  // don't forget, we will have one last match that didn't get to write out
  // in the while loop.
  fprintf(out_fp, "%02d-%02d:Purdue(%d)-%s(%d)\n", match_month, match_day,
    match_purdue_score, match_opp_name, match_opp_score);
  if (match_purdue_score > match_opp_score) {
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

  match m;
  reset_match(&m);

  int purdue_wins = 0;
  int purdue_losses = 0;

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

    if (!is_record_of_match(&r, &m)) {
      // this means the current match is done processing due to the change of record date stamp.
      // remember from the spec, all records of the same match are grouped together in the input
      // file.
      fprintf(out_fp, "%02d-%02d:Purdue(%d)-%s(%d)\n", m.month, m.day,
        m.purdue_score, m.opp_name, m.opp_score);
      if (m.purdue_score > m.opp_score) {
        purdue_wins++;
      }
      else {
        purdue_losses++;
      }
      reset_match(&m);
    }

    // If the record is from an existing match, then add it;
    // If the record is from a new match, then the prev if block already
    // resets the match so need to add the record to it.
    add_record_to_match(&r, &m);
  }

  if (m.year == INVALID_DATE) {
    // this means we never seen one valid record for this year.
    fclose(in_fp);
    in_fp = NULL;
    fclose(out_fp);
    out_fp = NULL;
    return NO_DATA_POINTS;
  }

  // don't forget, we will have one last match that didn't get to write out in the while loop.
  fprintf(out_fp, "%02d-%02d:Purdue(%d)-%s(%d)\n", m.month, m.day,
    m.purdue_score, m.opp_name, m.opp_score);
  if (m.purdue_score > m.opp_score) {
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
 * This function finds the most valuable player of a match at the given date
 * from the record file and returns the combined score of the MVP.
 */
double match_most_valuable_player_2(char *in_file, int year, int month, int day) {
  if ((year <= 0) || (month < 1) || (month > 12) || (day < 1) || (day > 30)) {
    return BAD_DATE;
  }

  FILE *in_fp = 0;
  in_fp = fopen(in_file, "r");
  if (in_fp == NULL) {
    return FILE_READ_ERR;
  }

  double highest_score = -1.0;
  while (true) {
    record r;
    int ret = read_record(in_fp, &r);

    if (ret == EOF) {
      break;
    } else if (ret != SUCCESS) {
      fclose(in_fp);
      in_fp = NULL;
      return ret;
    }

    if (r.year != year || r.month != month || r.day != day) {
      continue;
    }

    double combined_score = r.points + (1.5 * r.assists) + (2.0 * r.blocks) + (0.2 * r.min);
    if (combined_score > highest_score) {
      highest_score = combined_score;
    }
  }
  fclose(in_fp);
  in_fp = NULL;

  if (highest_score < 0) {
    return NO_DATA_POINTS;
  }
  return highest_score;
} /* match_most_valuable_player() */

/*
 * This function calculates and returns the average points of a player in the
 * total number of matches from the record file.
 */
double average_points_player_2(char *in_file, char *name) {
  FILE *in_fp = 0;
  in_fp = fopen(in_file, "r");
  if (in_fp == NULL) {
    return FILE_READ_ERR;
  }

  double points_scored = 0.0;
  int matches = 0;
  while (true) {
    record r;
    int ret = read_record(in_fp, &r);

    if (ret == EOF) {
      break;
    } else if (ret != SUCCESS) {
      fclose(in_fp);
      in_fp = NULL;
      return ret;
    }

    if (strcmp(r.player, name) != 0) {
      continue;
    }

    points_scored += r.points;
    matches++;
  }
  fclose(in_fp);
  in_fp = NULL;

  if (matches == 0) {
    return NO_DATA_POINTS;
  }
  return points_scored / matches;
} /* average_points_player() */

/*
 * This function finds Purdue's best winning game in a certain month and year
 * from the record file and returns Purdue's total score in that game.
 */
int purdue_best_winning_match_score_2(char *in_file, int year, int month) {
  if (year <= 0 || month < 1 || month > 12) {
    return BAD_DATE;
  }
  FILE *in_fp = 0;
  in_fp = fopen(in_file, "r");
  if (in_fp == NULL) {
    return FILE_READ_ERR;
  }

  match m;
  reset_match(&m);

  int max_purdue_score = -1;
  int max_diff = -1;

  while (true) {
    record r;
    int ret = read_record(in_fp, &r);

    if (ret == EOF) {
      break;
    } else if (ret != SUCCESS) {
      fclose(in_fp);
      in_fp = NULL;
      return ret;
    }

    // if year/month not match, or this record isn't of Purdue's player, then no need to proces.
    if (r.year != year || r.month != month || !is_purdue(r.team)) {
      continue;
    }

    if (!is_record_of_match(&r, &m)) {
      // this means the current match is done processing due to the change of record date stamp.
      // remember from the spec, all records of the same match are grouped together in the input
      // file.
      if (m.purdue_score > m.opp_score && m.purdue_score - m.opp_score > max_diff) {
        max_diff = m.purdue_score - m.opp_score;
        max_purdue_score = m.purdue_score;
      }
      reset_match(&m);
    }

    // If the record is from an existing match, then add it;
    // If the record is from a new match, then the prev if block already
    // resets the match so need to add the record to it.
    add_record_to_match(&r, &m);
  }
  fclose(in_fp);
  in_fp = NULL;

  // Because inside the while loop, we only process the records belonging to the year/month/Purdue
  // combo, and we only reset if we see a new record matching /year/month/Purdue, and immediately
  // adds it to a new match. In other words, the match record will never be empty, unless there
  // is truly no interested data in the file.
  if (m.year == INVALID_DATE) {
    // this means we never seen one valid record for Purdue for the year/month.
    return NO_DATA_POINTS;
  }

  // Out of the while loop, we'll always have one last match record to process because inside
  // the loop, there is no another interested match to finish the current one for processing.
  if (m.purdue_score - m.opp_score > max_diff) {
    max_diff = m.purdue_score - m.opp_score;
    max_purdue_score = m.purdue_score;
  }

  return max_purdue_score;
} /* purdue_best_winning_match_score() */

/*
 * This function determines and returns the month in which Purdue has its
 *  win rate from the record file.
 */
int purdue_best_month_2(char *in_file) {
  FILE *in_fp = 0;
  in_fp = fopen(in_file, "r");
  if (in_fp == NULL) {
    return FILE_READ_ERR;
  }

  match m;
  reset_match(&m);

  int games_per_month[13];
  memset(games_per_month, 0, sizeof(games_per_month));
  int purdue_wins_per_month[13];
  memset(purdue_wins_per_month, 0, sizeof(purdue_wins_per_month));

  while (true) {
    record r;
    int ret = read_record(in_fp, &r);

    if (ret == EOF) {
      break;
    } else if (ret != SUCCESS) {
      fclose(in_fp);
      in_fp = NULL;
      return ret;
    }

    if (!is_record_of_match(&r, &m)) {
      // this means the current match is done processing due to the change of record date stamp.
      // remember from the spec, all records of the same match are grouped together in the input
      // file.
      games_per_month[m.month]++;

      if (m.purdue_score > m.opp_score) {
        purdue_wins_per_month[m.month]++;
      }

      reset_match(&m);
    }

    // If the record is from an existing match, then add it;
    // If the record is from a new match, then the prev if block already
    // resets the match so need to add the record to it.
    add_record_to_match(&r, &m);
  }
  fclose(in_fp);
  in_fp = NULL;

  if (m.year != INVALID_DATE) {
    games_per_month[m.month]++;
    if (m.purdue_score > m.opp_score) {
      purdue_wins_per_month[m.month]++;
    }
  }

  double max_win_rate = -1.0;
  int best_month = -1;
  for (int i = 1; i <= 12; i++) {
    if (games_per_month[i] > 0) {
      double win_rate = (double)purdue_wins_per_month[i] / games_per_month[i];
      if (win_rate > max_win_rate) {
        max_win_rate = win_rate;
        best_month = i;
      }
    }
  }

  return best_month >= 0 ? best_month : NO_DATA_POINTS;
} /* purdue_best_month() */

/*
 * This function generates a comprehensive report of a requested Purdue player
 * including their name, total number of played games, overall statistics, and
 * number of games won from the record file.
 */
int generate_player_report_2(char *in_file, char *name, char *out_file) {
  FILE *in_fp = 0;
  in_fp = fopen(in_file, "r");
  if (in_fp == NULL) {
    return FILE_READ_ERR;
  }

  match m;
  reset_match(&m);

  bool player_in_game = false;
  int games_player_in = 0;
  int games_player_won = 0;
  double total_points = 0.0;
  double total_assists = 0.0;
  double total_blocks = 0.0;
  double total_minutes = 0.0;

  while (true) {
    record r;
    int ret = read_record(in_fp, &r);

    if (ret == EOF) {
      break;
    } else if (ret != SUCCESS) {
      fclose(in_fp);
      in_fp = NULL;
      return ret;
    }

    if (!is_record_of_match(&r, &m)) {
      // this means the current match is done processing due to the change of record date stamp.
      // remember from the spec, all records of the same match are grouped together in the input
      // file.
      if (player_in_game) {
        games_player_in++;
        if (m.purdue_score > m.opp_score) {
          games_player_won++;
        }
      }

      reset_match(&m);
      player_in_game = false;
    }

    // If the record is from an existing match, then add it;
    // If the record is from a new match, then the prev if block already
    // resets the match so need to add the record to it.
    add_record_to_match(&r, &m);

    if (is_purdue(r.team) && strcmp(r.player, name) == 0) {
      player_in_game = true;
      total_points += r.points;
      total_assists += r.assists;
      total_blocks += r.blocks;
      total_minutes += r.min;
    }
  }
  fclose(in_fp);
  in_fp = NULL;

  if (player_in_game) {
    games_player_in++;
    if (m.purdue_score > m.opp_score) {
      games_player_won++;
    }
  }

  if (games_player_in == 0) {
    return NO_DATA_POINTS;
  }

  FILE *out_fp = 0;
  out_fp = fopen(out_file, "w");
  if (out_fp == NULL) {
    return FILE_WRITE_ERR;
  }

  fprintf(out_fp, "Player: %s\n", name);
  fprintf(out_fp, "Games: %d\n", games_player_in);
  fprintf(out_fp, "Games Won: %d\n", games_player_won);
  fprintf(out_fp, "Points per Game: %.2f\n", total_points / games_player_in);
  fprintf(out_fp, "Assists per Game: %.2f\n", total_assists / games_player_in);
  fprintf(out_fp, "Blocks per Game: %.2f\n", total_blocks / games_player_in);
  fprintf(out_fp, "Average Minutes: %.2f\n", total_minutes / games_player_in);
  fclose(out_fp);
  out_fp = NULL;
  return SUCCESS;
} /* generate_player_report() */