#include <stdio.h>    // Για getchar, putchar, fprintf, stderr, EOF
#include <string.h>   // Για strcmp
#include <stdlib.h>   // Για strtod (μετατροπή string σε double), strtol
#include <math.h>     // Για sin, trunc, M_PI (για την generate)
#include <stdint.h>   // Για τύπους ακεραίων σταθερού μεγέθους (π.χ. int16_t)
/*
 * ===================================================================
 * ΒΟΗΘΗΤΙΚΕΣ ΣΥΝΑΡΤΗΣΕΙΣ (HELPER FUNCTIONS)
 * ===================================================================
 *
 * Αυτές οι συναρτήσεις διαχειρίζονται την ανάγνωση και εγγραφή
 * δεδομένων byte-byte, ακολουθώντας το πρότυπο little-endian.
 */

/**
 * @brief Διαβάζει 'n' bytes από το stdin και τα συνθέτει σε έναν
 * ακέραιο, ακολουθώντας το πρότυπο little-endian.
 * (Το λιγότερο σημαντικό byte έρχεται πρώτο).
 *
 * @param n Ο αριθμός των bytes (συνήθως 1, 2, ή 4).
 * @return Ο αριθμός που διαβάστηκε ως long long.
 * Επιστρέφει -1 αν συναντήσει EOF κατά την ανάγνωση.
 */
long long read_n_bytes_le(int n) {
    unsigned long long value = 0;
    for (int i = 0; i < n; i++) {
        int byte = getchar();
        if (byte == EOF) {
            return -1; // Σημαία σφάλματος/τέλους αρχείου
        }
        // Κάνουμε bitwise OR τον νέο byte στη σωστή θέση.
        // π.χ. byte 1: 0x11 -> value = 0x11
        //      byte 2: 0x22 -> value = 0x11 | (0x22 << 8) = 0x2211
        value |= ((unsigned long long)byte << (i * 8));
    }
    return (long long)value;
}

/**
 * @brief Γράφει έναν ακέραιο 'value' στο stdout σε 'n' bytes,
 * ακολουθώντας το πρότυπο little-endian.
 *
 * @param value Η τιμή προς εγγραφή.
 * @param n Ο αριθμός των bytes (συνήθως 1, 2, ή 4).
 */
void write_n_bytes_le(unsigned long long value, int n) {
    for (int i = 0; i < n; i++) {
        // Παίρνουμε το κατάλληλο byte με bitwise shift και AND
        // π.χ. value = 0x11223344, n = 4
        // i=0: (value >> 0) & 0xFF = 0x44
        // i=1: (value >> 8) & 0xFF = 0x33
        // ...
        putchar((value >> (i * 8)) & 0xFF);
    }
}

/**
 * @brief Διαβάζει 'n' bytes από το stdin και τα αγνοεί (δεν τα αποθηκεύει).
 *
 * @param n Ο αριθμός των bytes προς παράλειψη.
 * @return 0 για επιτυχία, -1 αν συναντήσει EOF.
 */
int skip_bytes(long long n) {
    for (long long i = 0; i < n; i++) {
        if (getchar() == EOF) {
            return -1;
        }
    }
    return 0;
}

/**
 * @brief Διαβάζει 'n' bytes από το stdin και τα γράφει απευθείας
 * στο stdout.
 *
 * @param n Ο αριθμός των bytes προς αντιγραφή.
 * @return 0 για επιτυχία, -1 αν συναντήσει EOF.
 */
int copy_bytes(long long n) {
    for (long long i = 0; i < n; i++) {
        int byte = getchar();
        if (byte == EOF) {
            return -1;
        }
        putchar(byte);
    }
    return 0;
}

/**
 * @brief Διαβάζει 'len' bytes, ελέγχει αν ταιριάζουν με το 'str',
 * και τα γράφει στο stdout *μόνο* αν ταιριάζουν.
 *
 * @param str Το αναμενόμενο string (π.χ. "RIFF").
 * @param len Το μήκος του string.
 * @return 1 για επιτυχία (ταίριασμα), 0 για σφάλμα (EOF ή αναντιστοιχία).
 */
int read_pass_and_check_string(const char* str, int len) {
    char buffer[4]; // Μικρός buffer για να κρατήσουμε τα bytes
    for (int i = 0; i < len; i++) {
        int byte = getchar();
        if (byte == EOF) {
            return 0; // Σφάλμα
        }
        buffer[i] = (char)byte;
        if (buffer[i] != str[i]) {
            return 0; // Σφάλμα
        }
    }
    // Αφού επιβεβαιώσαμε ότι ταιριάζουν, τα γράφουμε
    for (int i = 0; i < len; i++) {
        putchar(buffer[i]);
    }
    return 1; // Επιτυχία
}

/**
 * @brief Διαβάζει 'len' bytes και ελέγχει αν ταιριάζουν με το 'str'.
 * (Δεν γράφει τίποτα στο stdout).
 *
 * @param str Το αναμενόμενο string (π.χ. "RIFF").
 * @param len Το μήκος του string.
 * @return 1 για επιτυχία (ταίριασμα), 0 για σφάλμα (EOF ή αναντιστοιχία).
 */
int read_and_check_string(const char* str, int len) {
    for (int i = 0; i < len; i++) {
        int byte = getchar();
        if (byte == EOF || byte != str[i]) {
            return 0; // Σφάλμα
        }
    }
    return 1; // Επιτυχία
}

/*
 * ===================================================================
 * ΥΛΟΠΟΙΗΣΗ ΥΠΟΕΝΤΟΛΩΝ (SUBCOMMAND HANDLERS)
 * ===================================================================
 */

/**
 * @brief Υλοποιεί την υποεντολή 'info'.
 * Διαβάζει την κεφαλίδα, την επικυρώνει, και τυπώνει
 * τις πληροφορίες στο stdout.
 * Καταναλώνει ολόκληρη την είσοδο για να ελέγξει
 * για σφάλματα μεγέθους αρχείου.
 *
 * @return 0 για επιτυχία, 1 για σφάλμα.
 */
int handle_info() {
    long long size_of_file, size_of_format_chunk, wave_type_format, mono_stereo,
              sample_rate, bytes_per_sec, block_align, bits_per_sample, size_of_data;

    [cite_start]// 1. Έλεγχος "RIFF" [cite: 171]
    if (!read_and_check_string("RIFF", 4)) {
        fprintf(stderr, "Error! \"RIFF\" not found\n");
        return 1;
    }

    [cite_start]// 2. SizeOfFile [cite: 120]
    size_of_file = read_n_bytes_le(4);
    if (size_of_file == -1) { /* Σφάλμα EOF */ return 1; }
    printf("size of file: %lld\n", size_of_file);

    [cite_start]// 3. Έλεγχος "WAVE" [cite: 182]
    if (!read_and_check_string("WAVE", 4)) {
        fprintf(stderr, "Error! \"WAVE\" not found\n");
        return 1;
    }

    [cite_start]// 4. Έλεγχος "fmt " [cite: 185]
    if (!read_and_check_string("fmt ", 4)) {
        fprintf(stderr, "Error! \"fmt \" not found\n");
        return 1;
    }

    [cite_start]// 5. SizeOfFormatChunk (πρέπει να είναι 16) [cite: 121, 189]
    size_of_format_chunk = read_n_bytes_le(4);
    printf("size of format chunk: %lld\n", size_of_format_chunk);
    if (size_of_format_chunk != 16) {
        fprintf(stderr, "Error! size of format chunk should be 16\n");
        return 1;
    }

    [cite_start]// 6. WaveTypeFormat (πρέπει να είναι 1) [cite: 122, 195]
    wave_type_format = read_n_bytes_le(2);
    printf("WAVE type format: %lld\n", wave_type_format);
    if (wave_type_format != 1) {
        fprintf(stderr, "Error! WAVE type format should be 1\n");
        return 1;
    }

    [cite_start]// 7. MonoStereo (πρέπει να είναι 1 ή 2) [cite: 123, 202]
    mono_stereo = read_n_bytes_le(2);
    printf("mono/stereo: %lld\n", mono_stereo);
    if (mono_stereo != 1 && mono_stereo != 2) {
        fprintf(stderr, "Error! mono/stereo should be 1 or 2\n");
        return 1;
    }

    [cite_start]// 8. SampleRate [cite: 124]
    sample_rate = read_n_bytes_le(4);
    printf("sample rate: %lld\n", sample_rate);

    [cite_start]// 9. BytesPerSec [cite: 125]
    bytes_per_sec = read_n_bytes_le(4);
    printf("bytes/sec: %lld\n", bytes_per_sec);

    [cite_start]// 10. BlockAlign [cite: 126]
    block_align = read_n_bytes_le(2);
    printf("block alignment: %lld\n", block_align);

    [cite_start]// 11. BitsPerSample (πρέπει να είναι 8 ή 16) [cite: 127, 222]
    bits_per_sample = read_n_bytes_le(2);
    printf("bits/sample: %lld\n", bits_per_sample);
    if (bits_per_sample != 8 && bits_per_sample != 16) {
        fprintf(stderr, "Error! bits/sample should be 8 or 16\n");
        return 1;
    }

    // 12. Έλεγχοι Συσχέτισης
    [cite_start]// BytesPerSec = SampleRate * BlockAlign [cite: 211, 53]
    if (bytes_per_sec != sample_rate * block_align) {
        fprintf(stderr, "Error! bytes/second should be sample rate x block alignment\n");
        return 1;
    }
    [cite_start]// BlockAlign = BitsPerSample/8 * MonoStereo [cite: 232, 71]
    if (block_align != (bits_per_sample / 8) * mono_stereo) {
        fprintf(stderr, "Error! block alignment should be bits per sample / 8 x mono/stereo\n");
        return 1;
    }

    [cite_start]// 13. Έλεγχος "data" [cite: 243]
    if (!read_and_check_string("data", 4)) {
        fprintf(stderr, "Error! \"data\" not found\n");
        return 1;
    }

    [cite_start]// 14. SizeOfData [cite: 128]
    size_of_data = read_n_bytes_le(4);
    printf("size of data chunk: %lld\n", size_of_data);

    // 15. Έλεγχος Δεδομένων (Κατανάλωση)
    // Πρέπει να καταναλώσουμε τα δεδομένα για να ελέγξουμε για σφάλματα
    [cite_start]// μεγέθους αρχείου, όπως το 'insufficient data'. [cite: 255]

    // Καταναλώνουμε τα 'SizeOfData' bytes
    if (skip_bytes(size_of_data) == -1) {
        fprintf(stderr, "Error! insufficient data\n");
        return 1;
    }

    // 16. Έλεγχος "OtherData" και τέλους αρχείου
    // Τα bytes που απομένουν (OtherData) πρέπει να είναι:
    // SizeOfFile - (36 bytes κεφαλίδας μετά τα 8 πρώτα) - SizeOfData
    long long other_data_size = size_of_file - 36 - size_of_data;

    if (other_data_size < 0) {
        // Αυτό δείχνει ασυνέπεια στο SizeOfFile, αλλά το 'bad_sf'
        // ελέγχει αν υπάρχουν *περισσότερα* δεδομένα.
        other_data_size = 0;
    }
    
    // Καταναλώνουμε τα 'OtherData'
    if (skip_bytes(other_data_size) == -1) {
         // Χτύπησε EOF νωρίτερα απ' ό,τι περίμενε το SizeOfFile
        fprintf(stderr, "Error! bad file size (data ended before SizeOfFile)\n");
        return 1;
    }

    [cite_start]// 17. Έλεγχος για επιπλέον δεδομένα [cite: 266]
    if (getchar() != EOF) {
        fprintf(stderr, "Error! bad file size (found data past the expected end of file)\n");
        return 1;
    }

    return 0; // Επιτυχής έλεγχος
}

/**
 * @brief Υλοποιεί την υποεντολή 'rate'.
 * Αλλάζει την ταχύτητα αναπαραγωγής αλλάζοντας τα πεδία
 * SampleRate και BytesPerSec στην κεφαλίδα.
 * Αντιγράφει όλα τα υπόλοιπα δεδομένα (δείγματα ήχου, other data)
 * [cite_start]αυτούσια. [cite: 276, 280]
 *
 * @param rate_factor Ο παράγοντας αλλαγής ταχύτητας (π.χ. 0.5, 2.0).
 * @return 0 για επιτυχία, 1 για σφάλμα.
 */
int handle_rate(double rate_factor) {
    long long size_of_file, size_of_format_chunk, wave_type_format, mono_stereo,
              sample_rate, bytes_per_sec, block_align, bits_per_sample, size_of_data;

    // 1. "RIFF" - Έλεγχος και αντιγραφή
    if (!read_pass_and_check_string("RIFF", 4)) {
        fprintf(stderr, "Error! \"RIFF\" not found\n");
        return 1;
    }

    [cite_start]// 2. SizeOfFile - Ανάγνωση και αντιγραφή (δεν αλλάζει) [cite: 281, 291]
    size_of_file = read_n_bytes_le(4);
    if (size_of_file == -1) { return 1; }
    write_n_bytes_le(size_of_file, 4);

    // 3. "WAVE", "fmt " - Έλεγχος και αντιγραφή
    if (!read_pass_and_check_string("WAVE", 4)) {
        fprintf(stderr, "Error! \"WAVE\" not found\n");
        return 1;
    }
    if (!read_pass_and_check_string("fmt ", 4)) {
        fprintf(stderr, "Error! \"fmt \" not found\n");
        return 1;
    }

    // 4. SizeOfFormatChunk - Ανάγνωση, έλεγχος, αντιγραφή
    size_of_format_chunk = read_n_bytes_le(4);
    if (size_of_format_chunk != 16) {
        fprintf(stderr, "Error! size of format chunk should be 16\n");
        return 1;
    }
    write_n_bytes_le(size_of_format_chunk, 4);

    // 5. WaveTypeFormat - Ανάγνωση, έλεγχος, αντιγραφή
    wave_type_format = read_n_bytes_le(2);
    if (wave_type_format != 1) {
        fprintf(stderr, "Error! WAVE type format should be 1\n");
        return 1;
    }
    write_n_bytes_le(wave_type_format, 2);

    // 6. MonoStereo - Ανάγνωση, έλεγχος, αντιγραφή
    mono_stereo = read_n_bytes_le(2);
    if (mono_stereo != 1 && mono_stereo != 2) {
        fprintf(stderr, "Error! mono/stereo should be 1 or 2\n");
        return 1;
    }
    write_n_bytes_le(mono_stereo, 2);

    // 7. SampleRate - ΑΝΑΓΝΩΣΗ, ΑΛΛΑΓΗ, ΕΓΓΡΑΦΗ
    sample_rate = read_n_bytes_le(4);
    long long new_sample_rate = (long long)(sample_rate * rate_factor);
    write_n_bytes_le(new_sample_rate, 4);

    // 8. BytesPerSec - ΑΝΑΓΝΩΣΗ, ΑΛΛΑΓΗ, ΕΓΓΡΑΦΗ
    bytes_per_sec = read_n_bytes_le(4);
    // Το BlockAlign δεν αλλάζει, άρα το BytesPerSec αλλάζει αναλογικά
    // με το SampleRate.
    long long new_bytes_per_sec = (long long)(bytes_per_sec * rate_factor);
    write_n_bytes_le(new_bytes_per_sec, 4);

    // 9. BlockAlign - Ανάγνωση και αντιγραφή (δεν αλλάζει)
    block_align = read_n_bytes_le(2);
    write_n_bytes_le(block_align, 2);

    // 10. BitsPerSample - Ανάγνωση, έλεγχος, αντιγραφή
    bits_per_sample = read_n_bytes_le(2);
    if (bits_per_sample != 8 && bits_per_sample != 16) {
        fprintf(stderr, "Error! bits/sample should be 8 or 16\n");
        return 1;
    }
    write_n_bytes_le(bits_per_sample, 2);

    // 11. Έλεγχοι Συσχέτισης (με τις *παλιές* τιμές)
    if (bytes_per_sec != sample_rate * block_align) {
        fprintf(stderr, "Error! bytes/second should be sample rate x block alignment\n");
        return 1;
    }
    if (block_align != (bits_per_sample / 8) * mono_stereo) {
        fprintf(stderr, "Error! block alignment should be bits per sample / 8 x mono/stereo\n");
        return 1;
    }

    // 12. "data" - Έλεγχος και αντιγραφή
    if (!read_pass_and_check_string("data", 4)) {
        fprintf(stderr, "Error! \"data\" not found\n");
        return 1;
    }

    [cite_start]// 13. SizeOfData - Ανάγνωση και αντιγραφή (δεν αλλάζει) [cite: 281, 291]
    size_of_data = read_n_bytes_le(4);
    write_n_bytes_le(size_of_data, 4);

    // 14. Αντιγραφή Δειγμάτων Ήχου (SampleData)
    if (copy_bytes(size_of_data) == -1) {
        fprintf(stderr, "Error! insufficient data\n");
        return 1;
    }

    // 15. Αντιγραφή Υπόλοιπων Δεδομένων (OtherData)
    long long other_data_size = size_of_file - 36 - size_of_data;
    if (other_data_size < 0) { other_data_size = 0; }

    if (copy_bytes(other_data_size) == -1) {
        fprintf(stderr, "Error! bad file size (data ended before SizeOfFile)\n");
        return 1;
    }

    // 16. Έλεγχος για επιπλέον δεδομένα
    if (getchar() != EOF) {
        fprintf(stderr, "Error! bad file size (found data past the expected end of file)\n");
        return 1;
    }

    return 0;
}

/**
 * @brief Υλοποιεί την υποεντολή 'channel'.
 * Μετατρέπει ένα στερεοφωνικό αρχείο σε μονοφωνικό, κρατώντας
 * μόνο το αριστερό ('left') ή το δεξί ('right') κανάλι.
 * [cite_start]Αν το αρχείο είναι ήδη μονοφωνικό, απλά το αντιγράφει. [cite: 297]
 *
 * @param channel_name Το κανάλι που θα κρατηθεί ("left" ή "right").
 * @return 0 για επιτυχία, 1 για σφάλμα.
 */
int handle_channel(const char* channel_name) {
    long long size_of_file, size_of_format_chunk, wave_type_format, mono_stereo,
              sample_rate, bytes_per_sec, block_align, bits_per_sample, size_of_data;

    // Καθορίζουμε ποιο κανάλι θα κρατήσουμε (0=left, 1=right)
    int channel_to_keep = (strcmp(channel_name, "left") == 0) ? 0 : 1;

    // 1. "RIFF" - Έλεγχος και αντιγραφή
    if (!read_pass_and_check_string("RIFF", 4)) {
        fprintf(stderr, "Error! \"RIFF\" not found\n");
        return 1;
    }

    // 2. SizeOfFile - Ανάγνωση (ΔΕΝ το γράφουμε ακόμα, θα αλλάξει)
    size_of_file = read_n_bytes_le(4);

    // 3. "WAVE", "fmt " - Έλεγχος και αντιγραφή
    if (!read_pass_and_check_string("WAVE", 4)) { /* ... σφάλμα ... */ return 1; }
    if (!read_pass_and_check_string("fmt ", 4)) { /* ... σφάλμα ... */ return 1; }

    // 4. SizeOfFormatChunk - Ανάγνωση, έλεγχος, αντιγραφή
    size_of_format_chunk = read_n_bytes_le(4);
    if (size_of_format_chunk != 16) { /* ... σφάλμα ... */ return 1; }
    write_n_bytes_le(size_of_format_chunk, 4);

    // 5. WaveTypeFormat - Ανάγνωση, έλεγχος, αντιγραφή
    wave_type_format = read_n_bytes_le(2);
    if (wave_type_format != 1) { /* ... σφάλμα ... */ return 1; }
    write_n_bytes_le(wave_type_format, 2);

    // 6. MonoStereo - Ανάγνωση (ΔΕΝ το γράφουμε ακόμα)
    mono_stereo = read_n_bytes_le(2);
    if (mono_stereo != 1 && mono_stereo != 2) { /* ... σφάλμα ... */ return 1; }

    // 7. SampleRate - Ανάγνωση, αντιγραφή (δεν αλλάζει)
    sample_rate = read_n_bytes_le(4);
    write_n_bytes_le(sample_rate, 4);

    // 8. BytesPerSec - Ανάγνωση (ΔΕΝ το γράφουμε ακόμα)
    bytes_per_sec = read_n_bytes_le(4);

    // 9. BlockAlign - Ανάγνωση (ΔΕΝ το γράφουμε ακόμα)
    block_align = read_n_bytes_le(2);

    // 10. BitsPerSample - Ανάγνωση, έλεγχος, αντιγραφή
    bits_per_sample = read_n_bytes_le(2);
    if (bits_per_sample != 8 && bits_per_sample != 16) { /* ... σφάλμα ... */ return 1; }
    write_n_bytes_le(bits_per_sample, 2);

    // 11. Έλεγχοι Συσχέτισης (με τις *παλιές* τιμές)
    if (bytes_per_sec != sample_rate * block_align) { /* ... σφάλμα ... */ return 1; }
    if (block_align != (bits_per_sample / 8) * mono_stereo) { /* ... σφάλμα ... */ return 1; }

    // 12. "data" - Έλεγχος και αντιγραφή
    if (!read_pass_and_check_string("data", 4)) { /* ... σφάλμα ... */ return 1; }

    // 13. SizeOfData - Ανάγνωση (ΔΕΝ το γράφουμε ακόμα)
    size_of_data = read_n_bytes_le(4);

    // 14. Λογική: Αν είναι Μονοφωνικό, απλά αντιγράφουμε τα πάντα
    if (mono_stereo == 1) {
        // Γράφουμε τις τιμές που κρατήσαμε και δεν είχαμε γράψει
        write_n_bytes_le(size_of_file, 4);
        write_n_bytes_le(mono_stereo, 2);
        write_n_bytes_le(bytes_per_sec, 4);
        write_n_bytes_le(block_align, 2);
        write_n_bytes_le(size_of_data, 4);

        // Αντιγράφουμε τα δεδομένα
        if (copy_bytes(size_of_data) == -1) { /* ... σφάλμα ... */ return 1; }
        
        long long other_data_size = size_of_file - 36 - size_of_data;
        if (other_data_size < 0) { other_data_size = 0; }
        if (copy_bytes(other_data_size) == -1) { /* ... σφάλμα ... */ return 1; }

        if (getchar() != EOF) { /* ... σφάλμα ... */ return 1; }

        return 0; // Επιτυχία
    }

    // 15. Λογική: Αν είναι Στερεοφωνικό, κάνουμε μετατροπή
    
    // Υπολογισμός νέων τιμών κεφαλίδας
    long long new_mono_stereo = 1;
    long long new_block_align = block_align / 2;
    long long new_bytes_per_sec = bytes_per_sec / 2;
    long long new_size_of_data = size_of_data / 2;
    long long other_data_size = size_of_file - 36 - size_of_data;
    if (other_data_size < 0) { other_data_size = 0; }
    long long new_size_of_file = 36 + new_size_of_data + other_data_size;

    // Γράφουμε τις *νέες* τιμές
    write_n_bytes_le(new_size_of_file, 4);
    write_n_bytes_le(new_mono_stereo, 2);
    write_n_bytes_le(new_bytes_per_sec, 4);
    write_n_bytes_le(new_block_align, 2);
    write_n_bytes_le(new_size_of_data, 4);

    // 16. Επεξεργασία Δειγμάτων Ήχου
    int bytes_per_sample = bits_per_sample / 8;
    long long num_frames = size_of_data / block_align; // Αριθμός "καρέ" (L+R)

    for (long long i = 0; i < num_frames; i++) {
        // Κανάλι 0 (Left)
        if (channel_to_keep == 0) {
            // Το κρατάμε (αντιγραφή)
            if (copy_bytes(bytes_per_sample) == -1) { /* ... σφάλμα ... */ return 1; }
        } else {
            // Το πετάμε (παράλειψη)
            if (skip_bytes(bytes_per_sample) == -1) { /* ... σφάλμα ... */ return 1; }
        }

        // Κανάλι 1 (Right)
        if (channel_to_keep == 1) {
            // Το κρατάμε
            if (copy_bytes(bytes_per_sample) == -1) { /* ... σφάλμα ... */ return 1; }
        } else {
            // Το πετάμε
            if (skip_bytes(bytes_per_sample) == -1) { /* ... σφάλμα ... */ return 1; }
        }
    }

    // 17. Αντιγραφή OtherData
    if (copy_bytes(other_data_size) == -1) { /* ... σφάλμα ... */ return 1; }
    if (getchar() != EOF) { /* ... σφάλμα ... */ return 1; }

    return 0;
}

/**
 * @brief Υλοποιεί την υποεντολή 'volume' (Bonus).
 * Πολλαπλασιάζει την ένταση κάθε δείγματος με έναν παράγοντα.
 * Διαβάζει κάθε δείγμα, το τροποποιεί, και το ξαναγράφει.
 * Εφαρμόζει clamping (περιορισμό) για να μην ξεπεραστούν
 * τα όρια των 8-bit ή 16-bit.
 *
 * @param vol_factor Ο παράγοντας αλλαγής έντασης (π.χ. 0.125).
 * @return 0 για επιτυχία, 1 για σφάλμα.
 */
int handle_volume(double vol_factor) {
    long long size_of_file, size_of_format_chunk, wave_type_format, mono_stereo,
              sample_rate, bytes_per_sec, block_align, bits_per_sample, size_of_data;

    // Η κεφαλίδα δεν αλλάζει καθόλου, απλά την διαβάζουμε,
    // την επικυρώνουμε και την αντιγράφουμε.
    // (Ακολουθείται η ίδια λογική με το handle_rate, αλλά χωρίς αλλαγές)

    if (!read_pass_and_check_string("RIFF", 4)) { /* ... σφάλμα ... */ return 1; }
    size_of_file = read_n_bytes_le(4);
    if (size_of_file == -1) { return 1; }
    write_n_bytes_le(size_of_file, 4);
    if (!read_pass_and_check_string("WAVE", 4)) { /* ... σφάλμα ... */ return 1; }
    if (!read_pass_and_check_string("fmt ", 4)) { /* ... σφάλμα ... */ return 1; }
    size_of_format_chunk = read_n_bytes_le(4);
    if (size_of_format_chunk != 16) { /* ... σφάλμα ... */ return 1; }
    write_n_bytes_le(size_of_format_chunk, 4);
    wave_type_format = read_n_bytes_le(2);
    if (wave_type_format != 1) { /* ... σφάλμα ... */ return 1; }
    write_n_bytes_le(wave_type_format, 2);
    mono_stereo = read_n_bytes_le(2);
    if (mono_stereo != 1 && mono_stereo != 2) { /* ... σφάλμα ... */ return 1; }
    write_n_bytes_le(mono_stereo, 2);
    sample_rate = read_n_bytes_le(4);
    write_n_bytes_le(sample_rate, 4);
    bytes_per_sec = read_n_bytes_le(4);
    write_n_bytes_le(bytes_per_sec, 4);
    block_align = read_n_bytes_le(2);
    write_n_bytes_le(block_align, 2);
    bits_per_sample = read_n_bytes_le(2);
    if (bits_per_sample != 8 && bits_per_sample != 16) { /* ... σφάλμα ... */ return 1; }
    write_n_bytes_le(bits_per_sample, 2);
    if (bytes_per_sec != sample_rate * block_align) { /* ... σφάλμα ... */ return 1; }
    if (block_align != (bits_per_sample / 8) * mono_stereo) { /* ... σφάλμα ... */ return 1; }
    if (!read_pass_and_check_string("data", 4)) { /* ... σφάλμα ... */ return 1; }
    size_of_data = read_n_bytes_le(4);
    write_n_bytes_le(size_of_data, 4);

    // 14. Επεξεργασία Δειγμάτων Ήχου
    int bytes_per_sample = bits_per_sample / 8;
    long long num_samples = size_of_data / bytes_per_sample;

    for (long long i = 0; i < num_samples; i++) {
        long long sample_raw = read_n_bytes_le(bytes_per_sample);
        if (sample_raw == -1) { /* ... σφάλμα ... */ return 1; }

        double new_sample_double;
        long long new_sample_raw;

        if (bits_per_sample == 8) {
            [cite_start]// 8-bit: Unsigned [0, 255] [cite: 70]
            uint8_t sample_8 = (uint8_t)sample_raw;
            [cite_start]new_sample_double = trunc(sample_8 * vol_factor); [cite: 304]

            // Clamping (Περιορισμός)
            if (new_sample_double < 0.0) new_sample_double = 0.0;
            if (new_sample_double > 255.0) new_sample_double = 255.0;

            new_sample_raw = (long long)new_sample_double;

        } else {
            [cite_start]// 16-bit: Signed [-32768, 32767] [cite: 70]
            // Κάνουμε cast σε int16_t για να πάρει το σωστό πρόσημο
            int16_t sample_16 = (int16_t)sample_raw;
            [cite_start]new_sample_double = trunc((double)sample_16 * vol_factor); [cite: 304]

            // Clamping (Περιορισμός)
            if (new_sample_double < -32768.0) new_sample_double = -32768.0;
            if (new_sample_double > 32767.0) new_sample_double = 32767.0;

            // Το ξανακάνουμε cast σε (unsigned) int16_t (uint16_t)
            // για να το γράψουμε σωστά με τη write_n_bytes_le
            new_sample_raw = (long long)(int16_t)new_sample_double;
        }

        write_n_bytes_le(new_sample_raw, bytes_per_sample);
    }

    // 15. Αντιγραφή OtherData
    long long other_data_size = size_of_file - 36 - size_of_data;
    if (other_data_size < 0) { other_data_size = 0; }
    if (copy_bytes(other_data_size) == -1) { /* ... σφάλμα ... */ return 1; }
    if (getchar() != EOF) { /* ... σφάλμα ... */ return 1; }

    return 0;
}

/**
 * [cite_start]@brief Η συνάρτηση που παράγει τον ήχο για την 'generate'. [cite: 311]
 * Γράφει ολόκληρο το αρχείο .wav (κεφαλίδα και δείγματα)
 * στο stdout.
 */
void mysound(int dur, int sr, double fm, double fc, double mi, double amp) {
    
    // 1. Υπολογισμός τιμών κεφαλίδας
    [cite_start]long long mono_stereo = 1; [cite: 320]
    [cite_start]long long bits_per_sample = 16; [cite: 320]
    long long sample_rate = sr;
    long long block_align = (bits_per_sample / 8) * mono_stereo; // = 2
    long long bytes_per_sec = sample_rate * block_align;
    long long num_samples = (long long)dur * sr;
    long long size_of_data = num_samples * block_align;
    long long size_of_file = 36 + size_of_data;

    // 2. Εγγραφή Κεφαλίδας
    putchar('R'); putchar('I'); putchar('F'); putchar('F');
    write_n_bytes_le(size_of_file, 4);
    putchar('W'); putchar('A'); putchar('V'); putchar('E');
    putchar('f'); putchar('m'); putchar('t'); putchar(' ');
    write_n_bytes_le(16, 4); // SizeOfFormatChunk
    write_n_bytes_le(1, 2);  // WaveTypeFormat
    write_n_bytes_le(mono_stereo, 2);
    write_n_bytes_le(sample_rate, 4);
    write_n_bytes_le(bytes_per_sec, 4);
    write_n_bytes_le(block_align, 2);
    write_n_bytes_le(bits_per_sample, 2);
    putchar('d'); putchar('a'); putchar('t'); putchar('a');
    write_n_bytes_le(size_of_data, 4);

    // 3. Παραγωγή και Εγγραφή Δειγμάτων Ήχου
    for (long long i = 0; i < num_samples; i++) {
        // Υπολογισμός χρόνου 't'
        double t = (double)i / sr;

        [cite_start]// Εφαρμογή του μαθηματικού τύπου [cite: 310]
        double f_t = trunc(amp * sin(2 * M_PI * fc * t - mi * sin(2 * M_PI * fm * t)));
        
        // Clamping (Περιορισμός) στα όρια του 16-bit signed
        if (f_t > 32767.0) f_t = 32767.0;
        if (f_t < -32768.0) f_t = -32768.0;

        // Μετατροπή σε int16_t για εγγραφή
        int16_t sample = (int16_t)f_t;
        
        // Η write_n_bytes_le θα χειριστεί σωστά το (uint16_t)sample
        write_n_bytes_le(sample, 2);
    }
}

/**
 * @brief Υλοποιεί την υποεντολή 'generate' (Bonus).
 * Κάνει parse τα ορίσματα της γραμμής εντολών
 * (π.χ. --dur, --sr) και καλεί την mysound().
 *
 * @param argc Ο αριθμός των ορισμάτων από την main.
 * @param argv Ο πίνακας των ορισμάτων από την main.
 * @return 0 για επιτυχία, 1 για σφάλμα.
 */
int handle_generate(int argc, char* argv[]) {
    [cite_start]// 1. Ορισμός default τιμών [cite: 313-317]
    int dur = 3;
    int sr = 44100;
    double fm = 2.0;
    double fc = 1500.0;
    double mi = 100.0;
    double amp = 30000.0;

    // 2. Parsing ορισμάτων (από το argv[2] και μετά)
    for (int i = 2; i < argc; i++) {
        if (strcmp(argv[i], "--dur") == 0) {
            if (i + 1 < argc) {
                dur = (int)strtol(argv[i + 1], NULL, 10);
                i++; // Παραλείπουμε το επόμενο όρισμα (την τιμή)
            }
        } else if (strcmp(argv[i], "--sr") == 0) {
            if (i + 1 < argc) {
                sr = (int)strtol(argv[i + 1], NULL, 10);
                i++;
            }
        } else if (strcmp(argv[i], "--fm") == 0) {
            if (i + 1 < argc) {
                fm = strtod(argv[i + 1], NULL);
                i++;
            }
        } else if (strcmp(argv[i], "--fc") == 0) {
            if (i + 1 < argc) {
                fc = strtod(argv[i + 1], NULL);
                i++;
            }
        } else if (strcmp(argv[i], "--mi") == 0) {
            if (i + 1 < argc) {
                mi = strtod(argv[i + 1], NULL);
                i++;
            }
        } else if (strcmp(argv[i], "--amp") == 0) {
            if (i + 1 < argc) {
                amp = strtod(argv[i + 1], NULL);
                i++;
            }
        } else {
            fprintf(stderr, "Error! Unknown argument for 'generate': %s\n", argv[i]);
            return 1;
        }
    }

    // 3. Κλήση της συνάρτησης παραγωγής ήχου
    mysound(dur, sr, fm, fc, mi, amp);

    return 0;
}


/*
 * ===================================================================
 * ΚΥΡΙΑ ΣΥΝΑΡΤΗΣΗ (MAIN)
 * ===================================================================
 */

/**
 * @brief Κύρια συνάρτηση του προγράμματος.
 * Ελέγχει τα ορίσματα της γραμμής εντολών (argc, argv)
 * για να καθορίσει ποια υποεντολή θα εκτελεστεί.
 *
 * [cite_start]@return 0 για επιτυχία, 1 για σφάλμα. [cite: 102]
 */
int main(int argc, char* argv[]) {

    // Απαιτείται τουλάχιστον 1 όρισμα (η υποεντολή)
    if (argc < 2) {
        fprintf(stderr, "Error! No subcommand provided. Usage: ./soundwave <subcommand> [args...]\n");
        return 1;
    }

    // Παίρνουμε την υποεντολή (π.χ. "info")
    char* subcommand = argv[1];

    // Επιλογή της κατάλληλης συνάρτησης (handler)
    
    if (strcmp(subcommand, "info") == 0) {
        if (argc != 2) {
            fprintf(stderr, "Error! 'info' takes no arguments.\n");
            return 1;
        }
        return handle_info();
    
    } else if (strcmp(subcommand, "rate") == 0) {
        if (argc != 3) {
            fprintf(stderr, "Error! 'rate' requires one argument (rate factor).\n");
            return 1;
        }
        // Μετατροπή του 2ου ορίσματος (π.χ. "0.5") σε double
        double factor = strtod(argv[2], NULL);
        if (factor <= 0.0) {
            fprintf(stderr, "Error! Rate factor must be positive.\n");
            return 1;
        }
        return handle_rate(factor);
    
    } else if (strcmp(subcommand, "channel") == 0) {
        if (argc != 3) {
            fprintf(stderr, "Error! 'channel' requires one argument ('left' or 'right').\n");
            return 1;
        }
        if (strcmp(argv[2], "left") != 0 && strcmp(argv[2], "right") != 0) {
            fprintf(stderr, "Error! Channel must be 'left' or 'right'.\n");
            return 1;
        }
        return handle_channel(argv[2]);

    } else if (strcmp(subcommand, "volume") == 0) {
        if (argc < 3) {
             fprintf(stderr, "Error! 'volume' requires one argument (volume factor).\n");
            return 1;
        }
        double factor = strtod(argv[2], NULL);
         if (factor < 0.0) {
            fprintf(stderr, "Error! Volume factor must be non-negative.\n");
            return 1;
        }
        return handle_volume(factor);

    } else if (strcmp(subcommand, "generate") == 0) {
        // Η handle_generate θα κάνει η ίδια το parsing των επιπλέον ορισμάτων
        return handle_generate(argc, argv);
    
    } else {
        fprintf(stderr, "Error! Unknown subcommand: %s\n", subcommand);
        return 1;
    }

    // Αυτό δεν θα έπρεπε να συμβεί ποτέ
    return 1;
}