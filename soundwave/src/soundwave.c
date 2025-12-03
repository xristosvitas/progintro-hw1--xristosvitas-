#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h> // Απαραίτητο για trunc() και sin()

// Ορισμός της σταθεράς PI αν δεν είναι ήδη ορισμένη
#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

// Παγκόσμιος μετρητής bytes που έχουν διαβαστεί από το stdin.
// Απαραίτητος για τον έλεγχο 'bad file size' στην εντολή info και για τη χρήση
// μόνο της getchar() (μοναδικός τρόπος μέτρησης θέσης).
static long total_bytes_read = 0;

// ------------------------------------------------
// Βοηθητικές Συναρτήσεις για Ανάγνωση/Εγγραφή (Little-Endian)
// ------------------------------------------------

/**
 * Διαβάζει ένα byte από το stdin και ενημερώνει τον συνολικό μετρητή.
 * Η ΜΟΝΗ συνάρτηση που καλεί την getchar().
 * @return Το byte ως int (ή EOF).
 */
int read_byte_safe() {
    int byte = getchar();
    if (byte != EOF) {
        total_bytes_read++; // Αύξηση του μετρητή
    }
    return byte;
}

/**
 * Διαβάζει έναν ακέραιο 4-byte (uint32_t) little-endian.
 */
unsigned int read_le_uint32() {
    int byte1 = read_byte_safe();
    int byte2 = read_byte_safe();
    int byte3 = read_byte_safe();
    int byte4 = read_byte_safe();

    if (byte1 == EOF || byte2 == EOF || byte3 == EOF || byte4 == EOF) {
        return 0xFFFFFFFF; // Ενδεικτική τιμή σφάλματος
    }

    // Σύνθεση του 32-bit ακέραιου: little-endian (MSB << 24 | ... | LSB)
    unsigned int value = (unsigned int)byte1 |
                         ((unsigned int)byte2 << 8) |
                         ((unsigned int)byte3 << 16) |
                         ((unsigned int)byte4 << 24);

    return value;
}

/**
 * Διαβάζει έναν ακέραιο 2-byte (uint16_t) little-endian.
 */
unsigned short read_le_uint16() {
    int byte1 = read_byte_safe();
    int byte2 = read_byte_safe();

    if (byte1 == EOF || byte2 == EOF) {
        return 0xFFFF; // Ενδεικτική τιμή σφάλματος
    }

    // Σύνθεση του 16-bit ακέραιου: little-endian
    unsigned short value = (unsigned short)byte1 |
                           ((unsigned short)byte2 << 8);

    return value;
}

/**
 * Διαβάζει έναν ακέραιο 2-byte (int16_t) little-endian (για δείγματα ήχου).
 * Χρησιμοποιείται για την ανάγνωση 16-bit signed δειγμάτων.
 */
short read_le_int16() {
    return (short)read_le_uint16();
}

/**
 * Διαβάζει 4 bytes και τα συγκρίνει με ένα tag (π.χ. "RIFF").
 */
int check_tag(const char* expected_tag) {
    int i;
    for (i = 0; i < 4; i++) {
        int byte = read_byte_safe();
        if (byte == EOF || byte != expected_tag[i]) {
            return 0; // Αποτυχία ή EOF
        }
    }
    return 1;
}

/**
 * Εγγράφει 4 bytes στο stdout.
 */
void write_tag(const char* tag) {
    int i;
    for (i = 0; i < 4; i++) {
        putchar(tag[i]);
    }
}

/**
 * Εγγράφει έναν ακέραιο 4-byte (uint32_t) στο stdout, little-endian.
 */
void write_le_uint32(unsigned int value) {
    putchar(value & 0xFF);         // 1ο byte (LSB)
    putchar((value >> 8) & 0xFF);
    putchar((value >> 16) & 0xFF);
    putchar((value >> 24) & 0xFF); // 4ο byte (MSB)
}

/**
 * Εγγράφει έναν ακέραιο 2-byte (int16_t) στο stdout, little-endian.
 * Χρησιμοποιείται για την εγγραφή 16-bit signed δειγμάτων.
 */
void write_le_int16(short value) {
    // Η μετατροπή σε unsigned short είναι ασφαλής για να χρησιμοποιηθεί η write_le_uint16
    write_le_uint16((unsigned short)value);
}


// ------------------------------------------------
// Υποεντολή: info
// ------------------------------------------------

void handle_info() {
    // [1] RIFF Tag (4 bytes)
    if (!check_tag("RIFF")) {
        fprintf(stderr, "Error! \"RIFF\" not found\n");
        exit(1);
    }
    
    // [2] SizeOfFile (4 bytes)
    unsigned int size_of_file = read_le_uint32();
    if (size_of_file == 0xFFFFFFFF) {
        fprintf(stderr, "Error! Insufficient data (expected SizeOfFile)\n");
        exit(1);
    }
    printf("size of file: %u\n", size_of_file);

    // [3] WAVE Tag (4 bytes)
    if (!check_tag("WAVE")) {
        fprintf(stderr, "Error! \"WAVE\" not found\n");
        exit(1);
    }

    // [4] fmt Tag (4 bytes)
    if (!check_tag("fmt ")) { // Προσοχή στο κενό μετά το "fmt"
        fprintf(stderr, "Error! \"fmt\" not found\n");
        exit(1);
    }

    // [5] SizeOfFormatChunk (4 bytes)
    unsigned int size_of_format_chunk = read_le_uint32();
    if (size_of_format_chunk == 0xFFFFFFFF) {
        fprintf(stderr, "Error! Insufficient data (expected SizeOfFormatChunk)\n");
        exit(1);
    }
    printf("size of format chunk: %u\n", size_of_format_chunk);
    if (size_of_format_chunk != 16) {
        fprintf(stderr, "Error! size of format chunk should be 16\n");
        exit(1);
    }

    // [6] WAVETypeFormat (2 bytes)
    unsigned short wave_type_format = read_le_uint16();
    if (wave_type_format == 0xFFFF) {
        fprintf(stderr, "Error! Insufficient data (expected WAVETypeFormat)\n");
        exit(1);
    }
    printf("WAVE type format: %u\n", wave_type_format);
    if (wave_type_format != 1) { // Ελέγχουμε μόνο για PCM (1)
        fprintf(stderr, "Error! WAVE type format should be 1\n");
        exit(1);
    }

    // [7] MonoStereo (2 bytes)
    unsigned short mono_stereo = read_le_uint16();
    if (mono_stereo == 0xFFFF) {
        fprintf(stderr, "Error! Insufficient data (expected MonoStereo)\n");
        exit(1);
    }
    printf("mono/stereo: %u\n", mono_stereo);
    if (mono_stereo != 1 && mono_stereo != 2) {
        fprintf(stderr, "Error! mono/stereo should be 1 or 2\n");
        exit(1);
    }

    // [8] SampleRate (4 bytes)
    unsigned int sample_rate = read_le_uint32();
    if (sample_rate == 0xFFFFFFFF) {
        fprintf(stderr, "Error! Insufficient data (expected SampleRate)\n");
        exit(1);
    }
    printf("sample rate: %u\n", sample_rate);

    // [9] BytesPerSec (4 bytes)
    unsigned int bytes_per_sec = read_le_uint32();
    if (bytes_per_sec == 0xFFFFFFFF) {
        fprintf(stderr, "Error! Insufficient data (expected BytesPerSec)\n");
        exit(1);
    }
    printf("bytes/sec: %u\n", bytes_per_sec);

    // [10] BlockAlign (2 bytes)
    unsigned short block_align = read_le_uint16();
    if (block_align == 0xFFFF) {
        fprintf(stderr, "Error! Insufficient data (expected BlockAlign)\n");
        exit(1);
    }
    printf("block alignment: %u\n", block_align);

    // [11] BitsPerSample (2 bytes)
    unsigned short bits_per_sample = read_le_uint16();
    if (bits_per_sample == 0xFFFF) {
        fprintf(stderr, "Error! Insufficient data (expected BitsPerSample)\n");
        exit(1);
    }
    printf("bits/sample: %u\n", bits_per_sample);
    if (bits_per_sample != 8 && bits_per_sample != 16) {
        fprintf(stderr, "Error! bits/sample should be 8 or 16\n");
        exit(1);
    }

    // ************* Δευτερεύοντες Έλεγχοι Ορθότητας *************

    // [12] Έλεγχος BlockAlign: BlockAlign = BitsPerSample/8 * MonoStereo
    unsigned short expected_block_align = (bits_per_sample / 8) * mono_stereo;
    if (block_align != expected_block_align) {
        fprintf(stderr, "Error! block alignment should be bits per sample / 8 x mono/stereo\n");
        exit(1);
    }

    // [13] Έλεγχος BytesPerSec: BytesPerSec = SampleRate * BlockAlign
    unsigned int expected_bytes_per_sec = (unsigned int)sample_rate * block_align;
    if (bytes_per_sec != expected_bytes_per_sec) {
        fprintf(stderr, "Error! bytes/second should be sample rate x block alignment\n");
        exit(1);
    }

    // ************* Data Chunk *************

    // [14] data Tag (4 bytes)
    if (!check_tag("data")) {
        fprintf(stderr, "Error! \"data\" not found\n");
        exit(1);
    }

    // [15] SizeOfData (4 bytes)
    unsigned int size_of_data = read_le_uint32();
    if (size_of_data == 0xFFFFFFFF) {
        fprintf(stderr, "Error! Insufficient data (expected SizeOfData)\n");
        exit(1);
    }
    printf("size of data chunk: %u\n", size_of_data);

    // [16] Κατανάλωση των SampleData bytes
    int byte;
    for (unsigned int i = 0; i < size_of_data; i++) {
        byte = read_byte_safe();
        if (byte == EOF) {
            fprintf(stderr, "Error! insufficient data\n"); // Αν τελειώσουν τα bytes πριν το SizeOfData
            exit(1);
        }
    }

    // [17] Έλεγχος για "bad file size"
    // Το αναμενόμενο συνολικό μέγεθος είναι SizeOfFile + 8 (RIFF tag + SizeOfFile field)
    long expected_total_size = (long)size_of_file + 8;
    
    // Εάν ο μετρητής bytes είναι μεγαλύτερος από το αναμενόμενο, σημαίνει πλεονασματικά δεδομένα
    if (total_bytes_read > expected_total_size) {
        fprintf(stderr, "Error! bad file size (found data past the expected end of file)\n");
        exit(1);
    }
    
    // Κατανάλωση τυχόν OtherData (συνεχίζουμε μέχρι το EOF)
    while ((byte = read_byte_safe()) != EOF) {
        // Αγνόησε τυχόν OtherData
    }
}

// ------------------------------------------------
// Υποεντολή: rate
// ------------------------------------------------

void handle_rate(double fp_rate) {
    // [1] Ανάγνωση και Έλεγχοι (Όπως στην info)
    if (!check_tag("RIFF")) { fprintf(stderr, "Error! \"RIFF\" not found\n"); exit(1); }
    unsigned int size_of_file = read_le_uint32();
    if (size_of_file == 0xFFFFFFFF) { fprintf(stderr, "Error! Insufficient data (expected SizeOfFile)\n"); exit(1); }
    if (!check_tag("WAVE")) { fprintf(stderr, "Error! \"WAVE\" not found\n"); exit(1); }
    if (!check_tag("fmt ")) { fprintf(stderr, "Error! \"fmt\" not found\n"); exit(1); }
    
    unsigned int size_of_format_chunk = read_le_uint32();
    if (size_of_format_chunk != 16) { fprintf(stderr, "Error! size of format chunk should be 16\n"); exit(1); }
    unsigned short wave_type_format = read_le_uint16();
    if (wave_type_format != 1) { fprintf(stderr, "Error! WAVE type format should be 1\n"); exit(1); }
    unsigned short mono_stereo = read_le_uint16();
    if (mono_stereo != 1 && mono_stereo != 2) { fprintf(stderr, "Error! mono/stereo should be 1 or 2\n"); exit(1); }
    unsigned int sample_rate = read_le_uint32();
    unsigned int bytes_per_sec = read_le_uint32();
    unsigned short block_align = read_le_uint16();
    unsigned short bits_per_sample = read_le_uint16();
    if (bits_per_sample != 8 && bits_per_sample != 16) { fprintf(stderr, "Error! bits/sample should be 8 or 16\n"); exit(1); }
    
    if (block_align != (bits_per_sample / 8) * mono_stereo) { fprintf(stderr, "Error! block alignment is incorrect\n"); exit(1); }
    if (bytes_per_sec != sample_rate * block_align) { fprintf(stderr, "Error! bytes/second is incorrect\n"); exit(1); }

    if (!check_tag("data")) { fprintf(stderr, "Error! \"data\" not found\n"); exit(1); }
    unsigned int size_of_data = read_le_uint32();
    if (size_of_data == 0xFFFFFFFF) { fprintf(stderr, "Error! Insufficient data (expected SizeOfData)\n"); exit(1); }
    
    // [2] Τροποποίηση Πεδίων Κεφαλίδας
    
    // Νέα τιμή SampleRate: SampleRate * fp_rate (πολλαπλασιαστής ταχύτητας)
    unsigned int new_sample_rate = (unsigned int)((double)sample_rate * fp_rate);
    
    // Νέα τιμή BytesPerSec: New_SampleRate * BlockAlign
    unsigned int new_bytes_per_sec = (unsigned int)new_sample_rate * block_align;

    // [3] Εγγραφή Νέας Κεφαλίδας

    write_tag("RIFF");
    write_le_uint32(size_of_file); // SizeOfFile παραμένει ίδιο
    write_tag("WAVE");
    write_tag("fmt ");
    write_le_uint32(size_of_format_chunk);
    write_le_uint16(wave_type_format);
    write_le_uint16(mono_stereo);
    write_le_uint32(new_sample_rate);   // Νέα τιμή
    write_le_uint32(new_bytes_per_sec); // Νέα τιμή
    write_le_uint16(block_align);
    write_le_uint16(bits_per_sample);
    write_tag("data");
    write_le_uint32(size_of_data);

    // [4] Μεταφορά Δεδομένων (SampleData + OtherData)

    int byte;
    // Αντιγραφή SampleData (τα bytes ήχου παραμένουν ίδια)
    for (unsigned int i = 0; i < size_of_data; i++) {
        byte = read_byte_safe();
        if (byte == EOF) { fprintf(stderr, "Error! insufficient data\n"); exit(1); }
        putchar(byte); 
    }

    // Αντιγραφή τυχόν OtherData (μέχρι το EOF)
    while ((byte = read_byte_safe()) != EOF) {
        putchar(byte);
    }
}

// ------------------------------------------------
// Υποεντολή: channel
// ------------------------------------------------

void handle_channel(const char* channel_arg) {
    // Έλεγχος ορίσματος: 1=left, 0=right
    int keep_left = -1;
    if (strcmp(channel_arg, "left") == 0) keep_left = 1;
    else if (strcmp(channel_arg, "right") == 0) keep_left = 0;
    else { fprintf(stderr, "Error! 'channel' requires 'left' or 'right' as argument.\n"); exit(1); }
    
    // Ανάγνωση και Έλεγχοι (πρέπει να είναι stereo)
    if (!check_tag("RIFF")) { fprintf(stderr, "Error! \"RIFF\" not found\n"); exit(1); }
    unsigned int size_of_file = read_le_uint32();
    if (size_of_file == 0xFFFFFFFF) { fprintf(stderr, "Error! Insufficient data (expected SizeOfFile)\n"); exit(1); }
    if (!check_tag("WAVE")) { fprintf(stderr, "Error! \"WAVE\" not found\n"); exit(1); }
    if (!check_tag("fmt ")) { fprintf(stderr, "Error! \"fmt\" not found\n"); exit(1); }
    
    unsigned int size_of_format_chunk = read_le_uint32();
    if (size_of_format_chunk != 16) { fprintf(stderr, "Error! size of format chunk should be 16\n"); exit(1); }
    unsigned short wave_type_format = read_le_uint16();
    if (wave_type_format != 1) { fprintf(stderr, "Error! WAVE type format should be 1\n"); exit(1); }
    unsigned short mono_stereo = read_le_uint16();
    if (mono_stereo != 2) { fprintf(stderr, "Error! 'channel' can only be applied to stereo files (mono/stereo=2).\n"); exit(1); } // Πρέπει να είναι stereo
    unsigned int sample_rate = read_le_uint32();
    unsigned int bytes_per_sec = read_le_uint32();
    unsigned short block_align = read_le_uint16();
    unsigned short bits_per_sample = read_le_uint16();
    if (bits_per_sample != 8 && bits_per_sample != 16) { fprintf(stderr, "Error! bits/sample should be 8 or 16\n"); exit(1); }
    
    if (block_align != (bits_per_sample / 8) * mono_stereo) { fprintf(stderr, "Error! block alignment is incorrect\n"); exit(1); }
    if (bytes_per_sec != sample_rate * block_align) { fprintf(stderr, "Error! bytes/second is incorrect\n"); exit(1); }

    if (!check_tag("data")) { fprintf(stderr, "Error! \"data\" not found\n"); exit(1); }
    unsigned int size_of_data = read_le_uint32();
    if (size_of_data == 0xFFFFFFFF) { fprintf(stderr, "Error! Insufficient data (expected SizeOfData)\n"); exit(1); }
    
    // ************* Τροποποίηση Πεδίων Κεφαλίδας (Μετατροπή σε Mono) *************
    
    unsigned int bytes_per_sample = bits_per_sample / 8; // 1 ή 2 bytes
    
    // Όλα τα μεγέθη υποδιπλασιάζονται λόγω της αφαίρεσης ενός καναλιού
    unsigned short new_mono_stereo = 1; 
    unsigned short new_block_align = block_align / 2; 
    unsigned int new_bytes_per_sec = bytes_per_sec / 2; 
    unsigned int new_size_of_data = size_of_data / 2; 
    unsigned int new_size_of_file = size_of_file - (size_of_data - new_size_of_data); 
    
    // ************* Εγγραφή Νέας Κεφαλίδας *************

    write_tag("RIFF");
    write_le_uint32(new_size_of_file);
    write_tag("WAVE");
    write_tag("fmt ");
    write_le_uint32(size_of_format_chunk);
    write_le_uint16(wave_type_format);
    write_le_uint16(new_mono_stereo);   // Mono (1)
    write_le_uint32(sample_rate);
    write_le_uint32(new_bytes_per_sec);
    write_le_uint16(new_block_align);   // BlockAlign (π.χ. από 4 σε 2)
    write_le_uint16(bits_per_sample);
    write_tag("data");
    write_le_uint32(new_size_of_data);

    // ************* Μεταφορά Δεδομένων *************

    int byte;
    // Διαβάζουμε σε ζεύγη (αριστερό, δεξί) και γράφουμε μόνο το ζητούμενο
    for (unsigned int i = 0; i < size_of_data; i += block_align) {
        
        // 1. Διάβασμα ΑΡΙΣΤΕΡΟΥ δείγματος
        for (unsigned int b = 0; b < bytes_per_sample; b++) {
            byte = read_byte_safe();
            if (byte == EOF) { fprintf(stderr, "Error! insufficient data\n"); exit(1); }
            if (keep_left) {
                putchar(byte); // Κρατάμε το αριστερό
            }
        }
        
        // 2. Διάβασμα ΔΕΞΙΟΥ δείγματος
        for (unsigned int b = 0; b < bytes_per_sample; b++) {
            byte = read_byte_safe();
            if (byte == EOF) { fprintf(stderr, "Error! insufficient data\n"); exit(1); }
            if (!keep_left) {
                putchar(byte); // Κρατάμε το δεξί
            }
        }
    }
    
    // Αντιγραφή τυχόν OtherData (μέχρι το EOF)
    while ((byte = read_byte_safe()) != EOF) {
        putchar(byte);
    }
}

// ------------------------------------------------
// Υποεντολή: volume
// ------------------------------------------------

void handle_volume(double fp_multiplier) {
    // ... (Ανάγνωση και Έλεγχοι)
    
    if (!check_tag("RIFF")) { fprintf(stderr, "Error! \"RIFF\" not found\n"); exit(1); }
    unsigned int size_of_file = read_le_uint32();
    if (size_of_file == 0xFFFFFFFF) { fprintf(stderr, "Error! Insufficient data (expected SizeOfFile)\n"); exit(1); }
    if (!check_tag("WAVE")) { fprintf(stderr, "Error! \"WAVE\" not found\n"); exit(1); }
    if (!check_tag("fmt ")) { fprintf(stderr, "Error! \"fmt\" not found\n"); exit(1); }
    
    unsigned int size_of_format_chunk = read_le_uint32();
    if (size_of_format_chunk != 16) { fprintf(stderr, "Error! size of format chunk should be 16\n"); exit(1); }
    unsigned short wave_type_format = read_le_uint16();
    if (wave_type_format != 1) { fprintf(stderr, "Error! WAVE type format should be 1\n"); exit(1); }
    unsigned short mono_stereo = read_le_uint16();
    if (mono_stereo != 1 && mono_stereo != 2) { fprintf(stderr, "Error! mono/stereo should be 1 or 2\n"); exit(1); }
    unsigned int sample_rate = read_le_uint32();
    unsigned int bytes_per_sec = read_le_uint32();
    unsigned short block_align = read_le_uint16();
    unsigned short bits_per_sample = read_le_uint16();
    if (bits_per_sample != 8 && bits_per_sample != 16) { fprintf(stderr, "Error! volume only supports 8-bit or 16-bit samples.\n"); exit(1); }
    
    if (block_align != (bits_per_sample / 8) * mono_stereo) { fprintf(stderr, "Error! block alignment is incorrect\n"); exit(1); }
    if (bytes_per_sec != sample_rate * block_align) { fprintf(stderr, "Error! bytes/second is incorrect\n"); exit(1); }

    if (!check_tag("data")) { fprintf(stderr, "Error! \"data\" not found\n"); exit(1); }
    unsigned int size_of_data = read_le_uint32();
    if (size_of_data == 0xFFFFFFFF) { fprintf(stderr, "Error! Insufficient data (expected SizeOfData)\n"); exit(1); }

    // ************* Εγγραφή Κεφαλίδας (Αμετάβλητη) *************

    write_tag("RIFF"); write_le_uint32(size_of_file); write_tag("WAVE"); write_tag("fmt ");
    write_le_uint32(size_of_format_chunk); write_le_uint16(wave_type_format);
    write_le_uint16(mono_stereo); write_le_uint32(sample_rate);
    write_le_uint32(bytes_per_sec); write_le_uint16(block_align);
    write_le_uint16(bits_per_sample);
    write_tag("data"); write_le_uint32(size_of_data);

    // ************* Επεξεργασία Δειγμάτων *************

    unsigned int bytes_per_sample = bits_per_sample / 8;
    unsigned int total_samples = size_of_data / bytes_per_sample;
    int byte;

    for (unsigned int i = 0; i < total_samples; i++) {
        if (bits_per_sample == 8) {
            byte = read_byte_safe();
            if (byte == EOF) { fprintf(stderr, "Error! insufficient data\n"); exit(1); }
            
            // 8-bit: Μετατροπή σε signed (αφαίρεση του 128 offset)
            int centered_sample = byte - 128;
            
            // Εφαρμογή του πολλαπλασιαστή (έντασης)
            double new_centered = (double)centered_sample * fp_multiplier;
            
            // Ακέραιωση (trunc)
            int final_sample = (int)trunc(new_centered);
            
            // Περιορισμός (Clamping) στο εύρος [-128, 127]
            if (final_sample > 127) final_sample = 127;
            if (final_sample < -128) final_sample = -128;
            
            // Επαναφορά του offset και εγγραφή
            putchar(final_sample + 128);

        } else if (bits_per_sample == 16) {
            // 16-bit: Διάβασμα ως signed short
            short sample = read_le_int16();
            
            // Εφαρμογή του πολλαπλασιαστή (έντασης)
            double new_sample_f = (double)sample * fp_multiplier;
            
            // Ακέραιωση (trunc)
            int final_sample = (int)trunc(new_sample_f);
            
            // Περιορισμός (Clamping) στο εύρος [-32768, 32767]
            if (final_sample > 32767) final_sample = 32767;
            if (final_sample < -32768) final_sample = -32768;
            
            // Εγγραφή του signed 16-bit δείγματος
            write_le_int16((short)final_sample);
        }
    }
    
    // Αντιγραφή τυχόν OtherData (μέχρι το EOF)
    while ((byte = read_byte_safe()) != EOF) {
        putchar(byte);
    }
}


// ------------------------------------------------
// Υποεντολή: generate
// ------------------------------------------------

/**
 * Παράγει δείγματα ήχου με βάση τον τύπο FM/PM.
 */
void mysound(int dur, int sr, double fm, double fc, double mi, double amp) {
    // Υπολογισμός του συνολικού αριθμού δειγμάτων (διάρκεια * ρυθμός δειγματοληψίας)
    long total_samples = (long)dur * sr;
    
    // Τα δείγματα είναι 16-bit (2 bytes)
    unsigned int size_of_data = (unsigned int)total_samples * 2;
    
    // Το μέγεθος του αρχείου είναι το μέγεθος των δεδομένων + 36 bytes (το υπόλοιπο της κεφαλίδας)
    unsigned int size_of_file = size_of_data + 36;
    
    // Σταθερές παραμέτρους για generate: 16-bit, Mono (1)
    unsigned short mono_stereo = 1;
    unsigned short bits_per_sample = 16;
    unsigned short block_align = 2;
    unsigned int bytes_per_sec = (unsigned int)sr * 2; // sr * 1 * 2

    // ************* Εγγραφή ΝΕΑΣ Κεφαλίδας WAV *************
    write_tag("RIFF");
    write_le_uint32(size_of_file);
    write_tag("WAVE");
    write_tag("fmt ");
    write_le_uint32(16); // SizeOfFormatChunk: 16
    write_le_uint16(1);  // WAVETypeFormat: 1 (PCM)
    write_le_uint16(mono_stereo); // Mono (1)
    write_le_uint32(sr); // SampleRate (από όρισμα)
    write_le_uint32(bytes_per_sec);
    write_le_uint16(block_align);
    write_le_uint16(bits_per_sample); // 16 bits
    write_tag("data");
    write_le_uint32(size_of_data);

    // ************* Παραγωγή και Εγγραφή Δειγμάτων *************
    for (long i = 0; i < total_samples; i++) {
        // Υπολογισμός της στιγμής t (σε δευτερόλεπτα)
        double t = (double)i / sr;
        
        // Ο τύπος είναι: f(t) = trunc(amp * sin(2*PI*fc*t - mi * sin(2*PI*fm*t)))
        
        // Όρος διαμόρφωσης φάσης (Phase Modulation)
        double phase_mod = mi * sin(2.0 * M_PI * fm * t);
        
        // Συχνότητα φορέα (Carrier Frequency)
        double carrier = 2.0 * M_PI * fc * t;
        
        // Υπολογισμός του σήματος
        double signal_f = amp * sin(carrier - phase_mod);
        
        // Ακέραιωση (trunc)
        int sample_val = (int)trunc(signal_f);

        // Περιορισμός (Clamping) στο εύρος [-32768, 32767]
        if (sample_val > 32767) sample_val = 32767;
        if (sample_val < -32768) sample_val = -32768;
        
        // Εγγραφή του 16-bit δείγματος σε little-endian
        write_le_int16((short)sample_val);
    }
}

void handle_generate(int argc, char *argv[]) {
    // Ορισμός Προεπιλεγμένων Τιμών
    int dur = 2;
    int sr = 44100;
    double fm = 100.0;
    double fc = 1000.0;
    double mi = 100.0;
    double amp = 32767.0; // Μέγιστη τιμή 16-bit

    // Ανάλυση Ορισμάτων από τη γραμμή εντολών (ξεκινάει από το argv[2])
    if (argc > 2) dur = atoi(argv[2]);
    if (argc > 3) sr = atoi(argv[3]);
    if (argc > 4) fm = strtod(argv[4], NULL);
    if (argc > 5) fc = strtod(argv[5], NULL);
    if (argc > 6) mi = strtod(argv[6], NULL);
    if (argc > 7) amp = strtod(argv[7], NULL);
    
    // Έλεγχος ορίων
    if (dur <= 0 || sr <= 0) {
        fprintf(stderr, "Error! Duration and Sample Rate must be positive.\n");
        exit(1);
    }
    if (amp > 32767.0 || amp < 0.0) {
        fprintf(stderr, "Error! Amplitude must be between 0.0 and 32767.0.\n");
        exit(1);
    }
    
    mysound(dur, sr, fm, fc, mi, amp);
}


// ------------------------------------------------
// Κύρια Συνάρτηση
// ------------------------------------------------

int main(int argc, char *argv[]) {
    // Ρύθμιση μεγάλου buffer για stdin/stdout (8MB) για βελτιστοποίηση χρόνου/μνήμης
    // Αυτό είναι κρίσιμο για την τήρηση των περιορισμών χρόνου/μνήμης.
    setvbuf(stdin, NULL, _IOFBF, 1024 * 1024 * 8);
    setvbuf(stdout, NULL, _IOFBF, 1024 * 1024 * 8);

    if (argc < 2) {
        fprintf(stderr, "Error! Missing subcommand (info, rate, channel, volume, generate)\n");
        return 1;
    }

    const char *subcommand = argv[1];

    if (strcmp(subcommand, "info") == 0) {
        if (argc != 2) { fprintf(stderr, "Error! 'info' takes no arguments.\n"); return 1; }
        handle_info();
    } else if (strcmp(subcommand, "rate") == 0) {
        if (argc != 3) { fprintf(stderr, "Error! 'rate' requires one floating-point argument.\n"); return 1; }
        double fp_rate = strtod(argv[2], NULL);
        if (fp_rate <= 0) { fprintf(stderr, "Error! Rate multiplier must be positive.\n"); return 1; }
        handle_rate(fp_rate);
    } else if (strcmp(subcommand, "channel") == 0) {
        if (argc != 3) { fprintf(stderr, "Error! 'channel' requires one argument (left or right).\n"); return 1; }
        handle_channel(argv[2]);
    } else if (strcmp(subcommand, "volume") == 0) {
        if (argc != 3) { fprintf(stderr, "Error! 'volume' requires one floating-point argument.\n"); return 1; }
        double fp_multiplier = strtod(argv[2], NULL);
        if (fp_multiplier < 0) { fprintf(stderr, "Error! Volume multiplier cannot be negative.\n"); return 1; }
        handle_volume(fp_multiplier);
    } else if (strcmp(subcommand, "generate") == 0) {
        // Η generate μπορεί να πάρει έως 6 προαιρετικά ορίσματα (total 8 args)
        if (argc > 8) { fprintf(stderr, "Error! 'generate' takes up to 6 optional arguments.\n"); return 1; }
        handle_generate(argc, argv);
    } else {
        fprintf(stderr, "Error! Unknown subcommand: %s\n", subcommand);
        return 1;
    }

    fflush(stdout); // Εκτέλεση όλων των εκκρεμών εγγραφών στο stdout
    return 0;
}