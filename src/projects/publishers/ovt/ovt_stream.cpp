

#include "ovt_private.h"
#include "ovt_stream.h"
#include "ovt_session.h"

std::shared_ptr<OvtStream> OvtStream::Create(const std::shared_ptr<Application> application,
											 const StreamInfo &info,
											 uint32_t worker_count)
{
	auto stream = std::make_shared<OvtStream>(application, info);
	if(!stream->Start(worker_count))
	{
		return nullptr;
	}
	return stream;
}

OvtStream::OvtStream(const std::shared_ptr<Application> application,
					 const StreamInfo &info)
		: Stream(application, info)
{
}

OvtStream::~OvtStream()
{
	logtd("OvtStream(%d) has been terminated finally", GetId());
	Stop();
}

bool OvtStream::Start(uint32_t worker_count)
{
	logtd("OvtStream(%d) has started", GetId());
	_packetizer = std::make_shared<OvtPacketizer>(OvtPacketizerInterface::GetSharedPtr());

	/*
	"stream" :
	{
		"appName" : "app",
		"streamName" : "stream_720p",
		"tracks":
		[
			{
				"id" : 3291291,
				"codecId" : 32198392,
				"mediaType" : 0 | 1 | 2, # video | audio | data
				"timebase_num" : 90000,
				"timebase_den" : 90000,
				"bitrate" : 5000000,
				"startFrameTime" : 1293219321,
				"lastFrameTime" : 1932193921,
				"videoTrack" :
				{
					"framerate" : 29.97,
					"width" : 1280,
					"height" : 720
				},
				"audioTrack" :
				{
					"samplerate" : 44100,
					"sampleFormat" : "s16",
					"layout" : "stereo"
				}
			}
		]
	}
*/

	Json::Value 	json_root;
	Json::Value		json_tracks;

	json_root["appName"] = GetApplication()->GetName().CStr();
	json_root["streamName"] = GetName().CStr();

	for(auto &track_item : _tracks)
	{
		auto &track = track_item.second;

		Json::Value json_track;
		Json::Value json_video_track;
		Json::Value json_audio_track;

		json_track["id"] = track->GetId();
		json_track["codecId"] = static_cast<int8_t>(track->GetCodecId());
		json_track["mediaType"] = static_cast<int8_t>(track->GetMediaType());
		json_track["timebase_num"] = track->GetTimeBase().GetNum();
		json_track["timebase_den"] = track->GetTimeBase().GetDen();
		json_track["bitrate"] = track->GetBitrate();
		json_track["startFrameTime"] = track->GetStartFrameTime();
		json_track["lastFrameTime"] = track->GetLastFrameTime();

		json_video_track["framerate"] = track->GetFrameRate();
		json_video_track["width"] = track->GetWidth();
		json_video_track["height"] = track->GetHeight();

		json_audio_track["samplerate"] = track->GetSampleRate();
		json_audio_track["sampleFormat"] = static_cast<int8_t>(track->GetSample().GetFormat());
		json_audio_track["layout"] = static_cast<uint32_t>(track->GetChannel().GetLayout());

		json_track["videoTrack"] = json_video_track;
		json_track["audioTrack"] = json_audio_track;

		json_tracks.append(json_track);
	}

	json_root["tracks"] = json_tracks;

	_description = json_root;

	return Stream::Start(worker_count);
}

bool OvtStream::Stop()
{
	logtd("OvtStream(%d) has stopped", GetId());
	_packetizer.reset();

	return Stream::Stop();
}

void OvtStream::SendVideoFrame(const std::shared_ptr<MediaPacket> &media_packet)
{
	// Callback OnOvtPacketized()
	_packetizer->Packetize(media_packet->GetPts(), media_packet);
}

void OvtStream::SendAudioFrame(const std::shared_ptr<MediaPacket> &media_packet)
{
	// Callback OnOvtPacketized()
	_packetizer->Packetize(media_packet->GetPts(), media_packet);
}

bool OvtStream::OnOvtPacketized(std::shared_ptr<OvtPacket> &packet)
{
	// Broadcasting
	BroadcastPacket(OVT_PAYLOAD_TYPE_MEDIA_PACKET, packet->GetData());
	return true;
}

Json::Value& OvtStream::GetDescription()
{
	return _description;
}

bool OvtStream::RemoveSessionByConnectorId(int connector_id)
{
	auto sessions = GetAllSessions();

	for(const auto &item : sessions)
	{
		auto session = std::static_pointer_cast<OvtSession>(item.second);
		if(session->GetConnector()->GetId() == connector_id)
		{
			RemoveSession(session->GetId());
			return true;
		}
	}

	return false;
}