#ifndef MARC_ENCODER_H
#define MARC_ENCODER_H

#ifdef __cplusplus
extern "C" {
#endif


/*
 * Encode every MarcViiew record in input_path into one
 * sequential ISO 2709 output file.
 *
 * Returns:
 *     0   success
 *    -1   failure
 */
int marcviiew_encode_database(
    const char *input_path,
    const char *output_path
);


/*
 * Find one game by its MARC 001 / GAME_ID and encode only
 * that record into output_path.
 *
 * Example:
 *
 *     GAME_ID=007E01
 *
 * can be selected with:
 *
 *     "007E01"
 *
 * Returns:
 *     0   game found and encoded
 *    -1   failure or game not found
 */
int marcviiew_encode_game(
    const char *input_path,
    const char *output_path,
    const char *game_id
);


#ifdef __cplusplus
}
#endif

#endif