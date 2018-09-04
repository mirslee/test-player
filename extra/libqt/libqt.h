#pragma once

#include "vxconfig.h"
#include "qttype.h"

#define calloc _vxcalloc
#define malloc _vxmalloc
#define realloc _vxrealloc
#define free _vxfree

/*!
@function		oqt_init
@abstract		Initialize oqt_t data structure
@discussion	Not needed if you opened a file with oqt_open_* but may be useful
to do non-file related stuff with OpenQuicktime.
@param file	Pointer to oqt_t data structure
@result		0 if successful, 1 upon failure
*/
int oqt_init(oqt_t *file);


/*!
@function		oqt_close
@abstract		Close the file and delete all the objects
@param file	The file to close
@result		0 if successful, error number upon failure
*/
int oqt_uninit(oqt_t *file);



/*!
@function			oqt_read_headers
@abstract			Read all the information about the file into data structures
@discussion		Call straight after opening a file for reading.
Requires a MOOV atom be present in the file.
@param file		File to read information about
@result			If no MOOV atom exists return 1 else return 0.
*/
int oqt_read_headers(oqt_t *file);


/*!
@function			oqt_write_moviedata_init
@abstract			Start the movie data in output file
@discussion		Call just before data is written to file
(using oqt_encode_video etc.)
Starts the mdat atom.
@param file		File to start writing data to
@result			0 if successful, error number upon failure
*/
int oqt_write_moviedata_init(oqt_t *file);

/*!
@function			oqt_write_moviedata_end
@abstract			Finish writing the movie data
@discussion		Call once movie data has been written to file
and before closing the file. Ends the mdat atom
and writes the movie meta data (moov) atom.
@param file		File to start writing data to
@result			0 if successful, error number upon failure
*/
int oqt_write_moviedata_end(oqt_t *file);


/* ===================== Raw Data I/O */

/*!
@function			oqt_get_position
@abstract			Get the current postion in stream
@param file		File to get position of
@result			The the current byte position in stream
*/
__int64 oqt_get_position(oqt_t *file);

/*!
@function			oqt_set_position
@abstract			Set the current postion in stream
@param file		File to set position of
@param position	The position in file to change to
@result			0 if sucessful
*/
int oqt_set_position(oqt_t *file, __int64 position);

/*!
@function			oqt_get_length
@abstract			Get the length of the stream
@param file		File to get the length of
@result			-1 if unsucessful/unknown, or the that length of the file if sucessful
*/
__int64 oqt_get_length(oqt_t *file);

/*!
@function			oqt_write_video_data
@abstract			Write one raw (compressed) video frame
@discussion		This is very useful for editing applications
@param file		File to write data to
@param track		The number of the video track to work with
@param data		Buffer to write data from
@param size		Number of bytes to write from buffer
@param isAKeyFrame	1 if a key frame (I-frame), 0 otherwise
@result			Returns 0 if unsuccessful, 1 if successful
*/
int oqt_write_video_data(oqt_t *file, int track, BYTE *data, int size, int isAKeyFrame);


/*!
@function			oqt_get_video_track_count
@abstract			Get the number of video tracks in a file
@param file		File to count tracks in
@result			Number of video tracks in the file
*/
int oqt_get_video_track_count(oqt_t *file);

/*!
@function			oqt_get_audio_track_count
@abstract			Get the number of audio tracks in a file
@param file		File to count tracks in
@result			Number of audio tracks in the file
*/
int oqt_get_audio_track_count(oqt_t *file);


/*!
@function		oqt_get_info_value
@abstract		Get the value for an information field for file
@param file	The file to work with
@param code	The code of the field to get the value of
@result		The value of the field (string), or NULL if the field isn't defined
*/
const char* oqt_get_info_value(oqt_t *file, char *code);


/*!
@function		oqt_set_info_value
@abstract		Set the value for an information field
@discussion	Pass in an empty string or null to delete info field in file
@param file	The file to work with
@param code	The code of the field to set the value of
@param value	The value to assign to the field (string)
*/
void oqt_set_info_value(oqt_t *file, char *code, char *value);


/*!
@function		oqt_get_info_name
@abstract		Get the human readable name for a field
@param code	The code of the field to get the name of
@result		The name of the field (string), or NULL if the name of field is unknown
*/
const char* oqt_get_info_name(char *code);

/*!
@function		oqt_get_info_count
@abstract		Get the number of info fields in a file
@param file	The file to work with
@result		The number of fields defined
*/
int oqt_get_info_count(oqt_t *file);


/*!
@function		oqt_get_info_list
@abstract		Get an array of info field records
@param file	The file to get the info fields from
@result		The array of info field records
*/
oqt_udta_t* oqt_get_info_list(oqt_t *file);



/*!
@function		oqt_get_window_loc
@abstract		Get the location of a file's viewing window
@param file	The file to work with
@param x		Will contain the X location of the window upon return
@param y		Will contain the Y location of the window upon return
*/
void oqt_get_window_loc(oqt_t *file, short *x, short *y);

/*!
@function		oqt_set_window_loc
@abstract		Set the location of a file's viewing window
@param file	The file to work with
@param x		The new X location for the file's window
@param x		The new Y location for the file's window
*/
void oqt_set_window_loc(oqt_t *file, short x, short y);


/*!
@function			oqt_add_audio_track
@abstract			Set up tracks in a new file after opening and before writing
@discussion		Audio is stored two channels per quicktime track
@param file		File to add audio track to
@param channels	Number of audio channels to add
@param sample_rate Sample rate of the audio (Hz)
@param bits		Number for bits per Mono Sample
@param compressor	Four character code for the compressor to use
@result			Number of the audio track added
(Number less than 0 indicates an error occured)
*/
long oqt_add_audio_track(oqt_t *file,int channels,long sample_rate,int bits,char *compressor);

/*!
@function			oqt_add_video_track
@abstract			Add video tracks
@discussion		Video is stored one layer per quicktime track
@param file		File to add video tracks to
@param frame_w 	Width of frames (pixels)
@param frame_h	Height of frames (pixels)
@param framerate	The new framerate
@param frame_rate	Number of frames per second
@param compressor	Four character code for the compressor to use
@result			Number of the video track added
(Number less than 0 indicates an error occured)
*/
long oqt_add_video_track(oqt_t *file,int frame_w,int frame_h,float frame_rate,char *compressor);

/*!
@function			oqt_set_audio_param
@abstract			Set codec-dependant parameter for audio track
@param file		File to modify
@param track		Number of audio track to set parameter of
@param param		Name of the paramter to set
@param data		Value to set the parameter to
@result			Non-zero upon error
*/
int oqt_set_audio_param (oqt_t *file, int track,
														  const char* param, const void* data);

/*!
@function			oqt_set_video_param
@abstract			Set codec-dependant parameter for video track
@param file		File to modify
@param track		Number of video track to set parameter of
@param param		Name of the paramter to set
@param data		Value to set the parameter to
@result			Non-zero upon error
*/
int oqt_set_video_param (oqt_t *file, int track,
														  const char* param, const void* data);

/*!
@function			oqt_get_audio_param
@abstract			Get codec-dependant parameter for audio track
@param file		File to work with
@param track		Number of audio track to get parameter of
@param param		Name of the paramter to get
@param data		Pointer to memory to put result in
(must be at least OQT_MAX_PARAM_VALUE_LEN bytes long)
@result			Type of parameter (see OQT_PARAMETER_* defines) 
*/
int oqt_get_audio_param (oqt_t *file, int track,
														  const char* param, void* data);

/*!
@function			oqt_get_video_param
@abstract			Get codec-dependant parameter for video track
@param file		File to work with
@param track		Number of video track to get parameter of
@param param		Name of the paramter to get
@param data		Pointer to memory to put result in
(must be at least OQT_PARAMETER_MAX_VALUE_LEN bytes long)
@result			Type of parameter (see OQT_PARAMETER_* defines) 
*/
int oqt_get_video_param (oqt_t *file, int track,
														  const char* param, void* data);


/*!
@function			oqt_get_audio_length
@abstract			Get the number of samples in an audio track
@param file		File with audio track in
@param track		Number of the audio track to look at
@result			The the length of the audio track (samples)
*/
__int64 oqt_get_audio_length(oqt_t *file, int track);

/*!
@function			oqt_get_video_length
@abstract			Get the number of frames in a video track
@param file		File with video track in
@param track		Number of the video track to look at
@result			The the length of the video track (frames)
*/
__int64 oqt_get_video_length(oqt_t *file, int track);

/*!
@function			oqt_get_audio_position
@abstract			Get the current position in a audio track
@param file		File with audio track in
@param track		Number of the audio track to look at
@result			The current position in the audio track
*/
__int64 oqt_get_audio_position(oqt_t *file, int track);


/* set position of file descriptor relative to a track */
/*!
@function			oqt_set_audio_position
@abstract			Set position of file descriptor to postion relative to audio track and set file->data_file to the correct (external) resource.
@param file		File with audio track in
@param track		Number of the audio track to look at
@param sample		The sample number to seek to
@result			0 if successful
*/
int oqt_set_audio_position(oqt_t *file, int track, __int64 sample);

/* set position of file descriptor relative to a track */
/*!
@function			oqt_set_audio_position_to_chunk
@abstract			Set position of file descriptor to postion relative to audio track and set file->data_file to the correct (external) resource.
@param file		File with audio track in
@param track		Number of the audio track to look at
@param chunk		The chunk number to seek to
@result			0 if successful
*/
int
oqt_set_audio_position_to_chunk(oqt_t *file, int track, long chunk);

/*!
@function			oqt_get_video_position
@abstract			Get the current position in a video track
@param file		File with video track in
@param track		Number of the video track to look at
@result			The current frame position in the video track
*/
__int64 oqt_get_video_position(oqt_t *file, int track);

/*!
@function			oqt_set_video_position
@abstract			Set position of file descriptor to postion relative to video track and set file->data_file to the correct (external) resource.
@param file		File with audio track in
@param track		Number of the audio track to look at
@param frame		The frame number to seek to
@result			0 if successful
*/
int oqt_set_video_position(oqt_t *file, int track, __int64 frame);

/*!
@function			oqt_get_video_track_count
@abstract			Get the number of video tracks in a file
@param file		File to count tracks in
@result			Number of video tracks in the file
*/
int oqt_get_video_track_count(oqt_t *file);

/*!
@function			oqt_get_audio_track_count
@abstract			Get the number of audio tracks in a file
@param file		File to count tracks in
@result			Number of audio tracks in the file
*/
int oqt_get_audio_track_count(oqt_t *file);

/*!
@function			oqt_get_audio_samplerate
@abstract			Get the sample rate for an audio track
@param file		File with audio track
@param track		Number of the audio track to look at
@result			Sample rate of the audio track (Hz)
*/
long oqt_get_audio_samplerate(oqt_t *file, int track);


/*!
@function			oqt_get_audio_bits
@abstract			Get the number of bit per mono sample an audio track
@param file		File with audio track
@param track		Number of the audio track to look at
@result			The number of bits per mono sample
*/
int oqt_get_audio_bits(oqt_t *file, int track);

/*!
@function			oqt_get_audio_channels
@abstract			Get the number channels in an audio track
@param file		File with audio track
@param track		Number of the audio track to look at
@result			The number of channels in the track
*/
int oqt_get_audio_channels(oqt_t *file, int track);

/*!
@function			oqt_get_audio_compressor
@abstract			Get the compressor code of an audio track
@param file		File with audio track in
@param track		Number of the audio track to look at
@result			The four charecter code of the codec for the audio track 
*/
char* oqt_get_audio_compressor(oqt_t *file, int track);

/*!
@function			oqt_get_audio_codec
@abstract			Get information about codec for an audio track
@param file		File with audio track in
@param track		Number of the audio track to look at
@result			Codec information record
*/
const oqt_codec_info_t* oqt_get_audio_codec(oqt_t *file, int track);

/*!
@function			oqt_get_audio_channel_loc
@abstract			Get the quicktime track and channel that an audio channel belongs to
@param file		File with video track in
@param oqt_track	Pointer to integer which will contain track number upon completion
@param oqt_channel Pointer to integer which will contain track number upon completion
@param channel	Raw channel number
@result			0 if successful
*/
int oqt_get_audio_channel_loc(oqt_t *file, int *oqt_track, int *oqt_channel, int channel);



/*!
@function			oqt_get_video_width
@abstract			Get the width of frames in a video track
@param file		File with video track in
@param track		Number of the video track to look at
@result			The width of the video track frames in pixels
*/
int oqt_get_video_width(oqt_t *file, int track);

/*!
@function			oqt_get_video_height
@abstract			Get the height of frames in a video track
@param file		File with video track in
@param track		Number of the video track to look at
@result			The height of the video track frames in pixels
*/
int oqt_get_video_height(oqt_t *file, int track);

/*!
@function			oqt_get_video_depth
@abstract			Get the depth of frames in a video track
@param file		File with video track in
@param track		Number of the video track to look at
@result			The depth of the video track (in bits)
*/
int oqt_get_video_depth(oqt_t *file, int track);

/*!
@function			oqt_set_video_depth
@abstract			Set the depth of a track
@param file		File to modify
@param track	 	Number of the video track
@param depth		New depth for the track
*/
void oqt_set_video_depth(oqt_t *file, int track, int depth);

/*!
@function			oqt_get_video_framerate
@abstract			Get the frame rate of a video track
@param file		File with video track in
@param track		Number of the video track to look at
@result			The frame rate of the video track
*/
float oqt_get_video_framerate(oqt_t *file, int track);


/*!
@function			oqt_get_video_framesize
@abstract			Get the size of a video frame (in bytes)
@param file		File with video track in
@param track		Number of the video track to look at
@param frame		The video frame number to get the size of
@result			The size of the frame of video (in bytes)
*/
long oqt_get_video_framesize(oqt_t *file, int track, __int64 frame);


/*!
@function			oqt_get_video_sample_duration
@abstract			Get the duration of a particular video frame in 1/10000000 seconds unit
@param file		File with video track in
@param track		Number of the video track to look at
@result			The frame number we're interested in
*/
__int64 oqt_get_video_frame_duration(oqt_t *file, int track, __int64 frame_number);


/*!
@function			oqt_set_video_framerate
@abstract			Change framerate for all the video tracks
@param file		File to modify
@param framerate	The new framerate
*/
void oqt_set_video_framerate(oqt_t *file, float framerate);

/*!
@function			oqt_get_video_compressor
@abstract			Get the compressor code of a video track
@param file		File with video track in
@param track		Number of the video track to look at
@result			The four charecter code of the codec for the video track 
*/
char* oqt_get_video_compressor(oqt_t *file, int track);

/*!
@function			oqt_get_video_codec
@abstract			Get information about codec for a video track
@param file		File with video track in
@param track		Number of the video track to look at
@result			Codec information record
*/
const oqt_codec_info_t* oqt_get_video_codec(oqt_t *file, int track);

/*!
@function			oqt_get_video_codec
@abstract			Get information about codec for a video track
@param file		File with video track in
@param track		Number of the video track to look at
@result			Codec information record
*/
const oqt_codec_info_t* oqt_get_video_codec(oqt_t *file, int track);


/*!
@function			oqt_get_audio_chunks
@abstract			Get the number of chunks in an audio track
@param file		File with audio track in
@param track		Number of the audio track to look at
@result			The number of chunks of audio in the track
*/
long oqt_get_audio_chunks(oqt_t *file, int track);

/*!
@function			oqt_get_audio_chunksize
@abstract			Get size (in bytes) of a chunk of audio
@discussion		Chunk should be a number greater than 0
@param file		File with audio track in
@param track		Number of the audio track to look at
@param chunk		Number of the chunk to look at
@result			The number of bytes of raw data in the chunk
*/
long oqt_get_audio_chunksize(oqt_t *file, int track, long chunk);

/*!
@function			oqt_get_audio_chunk_samples
@abstract			Get the number of samples in a chunk
@discussion		Chunk should be a number greater than 0
@param file		File with audio track in
@param track		Number of the audio track to look at
@param chunk		Number of the chunk to look at
@result			The number of samples of audio in the chunk
*/
__int64 oqt_get_audio_chunk_samples(oqt_t *file, int track, long chunk);

/*!
@function			oqt_get_audio_frames_to_bytes
@abstract			Calculate the length in bytes of a number of frames of audio
@param file		File with audio track in
@param track		Number of the audio track to look at
@param frames		Number of frames of audio
@result			The length of those frames in bytes
*/
long oqt_get_audio_frames_to_bytes(oqt_t *file, int track, int frames);

/*!
@function			oqt_get_audio_framesize
@abstract			Get the size of an audio frame (in bytes)
@param file		File with audio track in
@param track		Number of the audio track to look at
@param frame		The frame number to get the size of
@result			The size of the frame of audio (in bytes)
*/
long oqt_get_audio_framesize(oqt_t *file, int track, __int64 frame);

/*!
@function			oqt_get_audio_sample_duration
@abstract			Get the duration of a particular audio frame in sample unit
@param file		File with audio track in
@param track		Number of the audio track to look at
@result			The frame number we're interested in
*/
long oqt_get_audio_frame_duration(oqt_t *file, int track, __int64 frame_number);

/*!
@function			oqt_get_audio_frames
@abstract			Get the number of audio frames in a track
@param file		File with audio track in
@param track		Number of the audio track to look at
@result			The number of audio frames
*/
__int64 oqt_get_audio_frames(oqt_t *file, int track);


/*!
@function			oqt_get_video_poster
@abstract			Get the frame number of the movies poster frame
@discussion		There is only one poster per movie but the track number
is needed because the frame number is relative to that track.
@param file		File with to get poster frame of
@param track		Number of the video track that the frame number is relative to
@result			The frame number, relative to 'track', for the movie
*/
__int64 oqt_get_video_poster(oqt_t *file, int track);

/*!
@function			oqt_set_video_poster
@abstract			Set the frame number of the movies poster frame
@discussion		There is only one poster per movie but the track number
is needed because the frame number is relative to that track.
@param file		File with to get poster frame of
@param track		Number of the video track that the frame number is relative to
@param frame		The number of the frame to change the movie poster to
*/
void oqt_set_video_poster(oqt_t *file, int track, __int64 frame);


/* ===================== Access to codecs. */


/*!
@function			oqt_supported_video
@abstract			Check to see if a video track's compressor is supported
@param file		File to check
@param track	 	Number of the video track to check
@result			1 if supported, 0 if unsupported
*/
int oqt_supported_video(oqt_t *file, int track);

/*!
@function			oqt_supported_audio
@abstract			Check to see if a audio track's compressor is supported
@param file		File to check
@param track	 	Number of the audio track to check
@result			1 if supported, 0 if unsupported
*/
int oqt_supported_audio(oqt_t *file, int track);

/*!
@function			oqt_reads_colormodel
@abstract			Check to see if the codec can generate the color model with no downsampling
@param file		File to check
@param track	 	Number of the video track to check
@param colormodel	The colour model to check for
@result			1 if supported, 0 if unsupported
*/
int oqt_reads_colormodel(oqt_t *file, int track, int colormodel );

/*!
@function			oqt_writes_colormodel
@abstract			Check to see if the codec can write the color model with no upsampling
@param file		File to check
@param track	 	Number of the video track to check
@param colormodel	The colour model to check for
@result			1 if supported, 0 if unsupported
*/
int oqt_writes_colormodel(oqt_t *file, int track, int colormodel );

/*!
@function			oqt_preferred_colormodel
@abstract			Get the codec's prefered colour model for track
@param file		File to check
@param track	 	Number of the video track to check
@result			The prefered colour model (see colormodels.h)
*/
int oqt_preferred_colormodel(oqt_t *file, int track);

/*!
@function			oqt_encode_video
@abstract			Encode a frame from frame buffer
@param file		File for encoding 
@param track	 	Track being encoded
@param color_model Colour model of data in buffer
@param row_pointers Buffer conaining raw video frame
@result			0 if successful
*/
int oqt_encode_video(oqt_t *file, int track,int color_model,BYTE **row_pointers );

/*!
@function			oqt_decode_video
@abstract			Decode a frame into a frame buffer
@param file		File for decoding 
@param track	 	Track being decoded
@param color_model Colour model for data put in buffer
@param row_pointers Buffer that will contain the decoded frame
@result			0 if successful
*/
int oqt_decode_video(oqt_t *file, int track,int color_model,BYTE **row_pointers);

/*!
@function			oqt_decode_audio
@abstract			Decode a audio samples into the buffer
@param file		File for decoding 
@param track		Audio track number to decode
@param output		Pointer to buffer for output as interleaved 2s complement integers
@param outputsize	Size of output buffer (bytes)
@result			Number of samples decoded
*/
long oqt_decode_audio(oqt_t *file,int track, BYTE *output, long outputsize );

/*!
@function			oqt_encode_audio
@abstract			Encode audio samples for the specified track
@discussion		Passing in no input (and samples set to zero)
will have the effect of flushing any internal
partially full buffers.
@param file		File for encoding 
@param track		Track to encode the audio to
@param input		Pointer to buffer for input as interleaved 2s complement integers
@param samples	Number of samples per channel
@result			Number of samples encoded
*/
long oqt_encode_audio(oqt_t *file,int track, BYTE *input, long samples );



/* ===================== Codec Loading and Listing. */


/*!
@function			 oqt_list_codec_params
@abstract			 List the available paramters for a codec
@param codec		 Pointer to codec information record
@param o_numparams Point t to an integer which will contain the number
of parameters the codec supports when the function returns.
@result			 Array of parameter description records.
May be a null pointer if codec has no parameters.
*/
const oqt_codec_param_t*
oqt_list_codec_params (const oqt_codec_info_t *codec, int *o_numparams);


/*!
@function			oqt_list_audio_codecs
@abstract			Get an array of pointers to information about audio codecs
@result			Array of pointers to codec information
*/
const oqt_codec_info_t** oqt_list_audio_codecs(void);

/*!
@function			oqt_list_video_codecs
@abstract			Get an array of pointers to information about video codecs
@result			Array of pointers to codec information
*/
const oqt_codec_info_t** oqt_list_video_codecs(void);

/*!
@function			oqt_count_video_codecs
@abstract			Get the number of video codecs registered
@result			Number of video codecs
*/
int oqt_count_video_codecs(void);

/*!
@function			oqt_count_audio_codecs
@abstract			Get the number of audio codecs registered
@result			Number of audio codecs
*/
int oqt_count_audio_codecs(void);




/* ===================== Miscellaneous. */


/*!
@function			oqt_dump
@abstract			Dump the file structures for a file to stdout
@param file		File to be dump information about
@result			Non-zero upon failure
*/
int oqt_dump(oqt_t *file);

/*!
@function			oqt_make_streamable
@abstract			Make a file streamable
@discussion  		Re-arranges atoms in the quicktime file to that the movie
can be played as it downloads. Does not hint the file.
@param in_file	Input file already opened with read privillages
@param out_file	Output file already opened with write privillages
@result			0 if successful, 1 upon failure
*/
int oqt_make_streamable(oqt_t *in_file, oqt_t *out_file);

/*!
@function			oqt_make_streamable_revisited
@abstract			Make a file streamable
@discussion  		Re-arranges atoms in the quicktime file to that the movie
can be played as it downloads. Does not hint the file.
@param file		file already opened with read/write privillages
@result			0 if successful, 1 upon failure
*/
int oqt_make_streamable_revisited(oqt_t *file);

/*!
@function			oqt_set_preload
@abstract			Specify whether to read contiguously or not.
@param file		File to set preload for
@param preload	Number of bytes to read ahead
*/
void oqt_set_preload(oqt_t *file, __int64 preload);

/*!
@function			oqt_get_video_keyframe_before
@abstract			Get position of the first keyframe before 'frame'
@discussion  		There is one keyframe table for each track
@param file		File to work with
@param track		The number of the video track to work with
@param frame		The number of the frame in the track to check before
*/
__int64 oqt_get_video_keyframe_before(oqt_t *file, int track, __int64 frame);

/*!
@function			oqt_get_video_keyframe_after
@abstract			Get position of the first keyframe after 'frame'
@discussion  		There is one keyframe table for each track
@param file		File to work with
@param track		The number of the video track to work with
@param frame		The number of the frame in the track to check after
*/
__int64 oqt_get_video_keyframe_after(oqt_t *file, int track, __int64 frame);

/*!
@function			oqt_insert_video_keyframe
@abstract			Insert the position keframe into the keyframe tables
@discussion  		There is one keyframe table for each track
@param file		File to work with
@param track		The number of the video track to work with
@param frame		Position of keyframe to add
*/
void oqt_insert_video_keyframe(oqt_t *file, int track, __int64 frame);


/* ===================== Raw Data I/O */

/*!
@function			oqt_get_position
@abstract			Get the current postion in stream
@param file		File to get position of
@result			The the current byte position in stream
*/
__int64 oqt_get_position(oqt_t *file);

/*!
@function			oqt_set_position
@abstract			Set the current postion in stream
@param file		File to set position of
@param position	The position in file to change to
@result			0 if sucessful
*/
int oqt_set_position(oqt_t *file, __int64 position);

/*!
@function			oqt_get_length
@abstract			Get the length of the stream
@param file		File to get the length of
@result			-1 if unsucessful/unknown, or the that length of the file if sucessful
*/
__int64 oqt_get_length(oqt_t *file);


/*!
@function			oqt_write_data
@abstract			Write 'size' bytes from buffer to stream
@param file		File to write data to
@param data		Buffer to write data from
@param size		Number of bytes to write from buffer
@result			Returns 0 if unsuccessful, 1 if successful
*/
int oqt_write_data(oqt_t *file, char *data, int size);

/*!
@function			oqt_read_data
@abstract			Read 'size' bytes from stream into buffer
@param file		File to read from
@param data		Buffer to put data into
@param size		Number of bytes to read
@result			Returns 0 if unsuccessful, 1 if successful
*/
int oqt_read_data(oqt_t *file, char *data, __int64 size);


/*!
@function			oqt_write_video_data
@abstract			Write one raw (compressed) video frame
@discussion		This is very useful for editing applications
@param file		File to write data to
@param track		The number of the video track to work with
@param data		Buffer to write data from
@param size		Number of bytes to write from buffer
@param isAKeyFrame	1 if a key frame (I-frame), 0 otherwise
@result			Returns 0 if unsuccessful, 1 if successful
*/
int oqt_write_video_data(oqt_t *file, int track, BYTE *data, int size, int isAKeyFrame);


/*!
@function			oqt_write_audio_data
@abstract			Write one raw (compressed) audio sample
@discussion		This is very useful for editing applications
@param file		File to write data to
@param track		The number of the audio track to work with
@param data		Buffer to write data from
@param size		Number of bytes to write from buffer
@param num_samples    	Number of samples this buffer corresponds to
@param num_frames			Number of frames this buffer corresponds to
@param frame_size_array	The size of each of the frames in the chunk
@result					Returns 0 if unsuccessful, 1 if successful
*/
int oqt_write_audio_data(oqt_t *file, int track, BYTE *output, int bytes, int num_samples, int num_frames, long* frame_size_array);


/*!
@function			oqt_get_mp4_video_decoder_config
@abstract			return the Decoder Config of a video track
@discussion		This is usefull for mp4 tools 
@param file		File to read from
@param track		The number of the video track
@param ppBuf		Buffer containing the DecoderConfig
@param pBufSize	Size of the DecoderConfig
@result			Returns 0 if unsuccessful, 1 if successful
*/
int oqt_get_mp4_video_decoder_config(oqt_t *file, int track, BYTE** ppBuf, int* pBufSize);


/*!
@function			oqt_set_mp4_video_decoder_config
@abstract			Set the Decoder Config of a video track
@discussion		This is usefull for mp4 tools 
@param file		File to write data to
@param track		The number of the video track
@param ppBuf		Buffer containing the DecoderConfig
@param pBufSize	Size of the DecoderConfig
@result			Returns 0 if unsuccessful, 1 if successful
*/
int oqt_set_mp4_video_decoder_config(oqt_t *file, int track, BYTE* pBuf, int bufSize);


/*!
@function			oqt_get_mp4_audio_decoder_config
@abstract			Return the Decoder Config of a audio track
@discussion		This is usefull for mp4 tools
@param file		File to read from
@param track		The number of the audio track
@param ppBuf		Buffer containing the DecoderConfig
@param pBufSize	Size of the DecoderConfig
@result			Returns 0 if unsuccessful, 1 if successful
*/
int oqt_get_mp4_audio_decoder_config(oqt_t *file, int track, BYTE** ppBuf, int* pBufSize);


/*!
@function			oqt_set_mp4_audio_decoder_config
@abstract			Set the Decoder Config of a audio track
@discussion		This is usefull for mp4 tools
@param file		File to write data to
@param track		The number of the audio track
@param ppBuf		Buffer containing the DecoderConfig
@param pBufSize	Size of the DecoderConfig
@result			Returns 0 if unsuccessful, 1 if successful
*/
int oqt_set_mp4_audio_decoder_config(oqt_t *file, int track, BYTE* pBuf, int bufSize);


/*!
@function			oqt_get_iod_audio_profile_level
@abstract			Get the IOD Audio Profile of the movie
@discussion		This is usefull for mp4 tools
@param file		File to write to
@result			Returns the audio profil
*/
int oqt_get_iod_audio_profile_level(oqt_t *file);


/*!
@function			oqt_set_iod_audio_profile_level
@abstract			Set the IOD Audio Profile of the movie
@discussion		This is usefull for mp4 tools
@param file		File to read from
@param id			the new Audio Profile
@result			Returns 0 if OK
*/
int oqt_set_iod_audio_profile_level(oqt_t *file, int id);


/*!
@function			oqt_get_iod_video_profile_level
@abstract			Get the IOD Video Profile of the movie
@discussion		This is usefull for mp4 tools
@param file		File to write to
@result			Returns the video profil
*/
int oqt_get_iod_video_profile_level(oqt_t *file);


/*!
@function			oqt_set_iod_video_profile_level
@abstract			Set the IOD Video Profile of the movie
@discussion		This is usefull for mp4 tools
@param file		File to read from
@param id			the new Video Profile
@result			Returns 0 if OK
*/
int oqt_set_iod_video_profile_level(oqt_t *file, int id);








/*!
@function		oqt_write_video_reference
@abstract		Write a reference entry for one video frame
@discussion		This is very useful for editing applications
@param file		File to write reference to (no data is writen)
@param track		The number of the video track to work with
@param url		URL of the referenced data source
@param offset		Byte offset into the referenced file
@param size		Size of this frame in bytes
@param isAKeyFrame	1 if a key frame (I-frame), 0 otherwise
@result		Returns 0 if unsuccessful, 1 if successful
*/
int oqt_write_video_reference(oqt_t *file,int track,char *url,__int64 offset,int bytes,int IsAKeyFrame);


/*!
@function		oqt_write_audio_reference
@abstract		Write a reference entry for an audio chunk
@discussion		This is very useful for editing applications
@param file		File to write reference to (no data is writen)
@param track		The number of the audio track to work with
@param url		URL of the referenced data source
@param offset		Byte offset into the referenced file
@param bytes		Number of bytes this chunk
@param num_samples    Number of samples this buffer corresponds to
@result		Returns 0 if unsuccessful, 1 if successful
*/
int oqt_write_audio_reference(oqt_t *file,int track,char *url,__int64 offset,int bytes,int num_samples);

/*!
@function			oqt_register_codec
@abstract			Add a codec to the registry
@discussion		'info' should be created using
oqt_allocate_audio_codec or oqt_allocate_video_codec.
Should only be called by plugins that register more than
one codec. The primary codec in plugins is registered automatically.
@param info		Pointer to a valid codec information record
@result			Returns 0 if registering was successful
*/
int oqt_register_codec( oqt_codec_info_t* info );


/* In codec_stub.c */
/*!
@function			oqt_allocate_audio_codec
@abstract			Allocate memory for audio codec and set defaults
@result			Returns pointer to the allocated memory
*/
oqt_audio_codec_t* oqt_allocate_audio_codec(void);

/*!
@function			oqt_allocate_video_codec
@abstract			Allocate memory for video codec and set defaults
@result			Returns pointer to the allocated memory
*/
oqt_video_codec_t* oqt_allocate_video_codec(void);


