<?php
/**
 * Playlist Item
 *
 * Copyright 2009 Frederick Ding<br />
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 *
 * You may obtain a copy of the License at
 * http://www.apache.org/licenses/LICENSE-2.0
 * or the full licensing terms for this project at
 * http://code.google.com/p/display-ui/wiki/License
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *
 * @author Frederick
 * @license http://code.google.com/p/display-ui/wiki/License Apache License 2.0
 * @version $Id$
 */
/**
 * A data structure for a playlist item
 */
class Api_Model_PlaylistItem
{
	/**
	 * A single image in JPEG or PNG format
	 */
	const IMAGE_TYPE = 1;
	/**
	 * An unimplemented type of item that is a bullet-point text slide
	 */
	const TEXT_TYPE = 2;
	/**
	 * A single video file in a supported format
	 */
	const VIDEO_TYPE = 3;
	/**
	 * A type of item that is an archive
	 * containing a set of images (each valid in the IMAGE_TYPE)
	 * to be played together.
	 */
	const IMAGESHOW_TYPE = 4;
	/**
	 * An unimplemented type of item that is a PowerPoint file to be played
	 * on a compatible Windows client with PowerPoint installed.
	 */
	const POWERPOINT_TYPE = 5;
	/**
	 * Item type, one of the above
	 * @var int
	 */
	private $type = 0;
	/**
	 * How long to show the item
	 * @var int
	 */
	private $duration = 20;
	private $filename = '';
	/**
	 * The ID of the item from the dui_media table
	 * @var int Unsigned integer
	 */
	private $media_id = 0;
	/**
	 * Builds a playlist item with the specified type and content
	 * @param int $media_id
	 * @param int $type
	 * @param array $content
	 * @param int $duration
	 * @return Api_Model_PlaylistItem fluent interface
	 */
	public function __construct ($media_id, $type, $content, $duration = 20)
	{
		$this->media_id = (int) $media_id;
		if (1 <= $type && $type <= 5) {
			$this->type = (int) $type;
		} else
			// throw new Exception('Bad type for Api_Playlist_Item', 100);
			die('Bad type for Api_Playlist_Item.');
		$this->duration = $duration;
		if ($this->type == self::IMAGE_TYPE || $this->type == self::VIDEO_TYPE ||
		 $this->type == self::POWERPOINT_TYPE) {
			$this->filename = $content['filename'];
		} elseif ($this->type == self::TEXT_TYPE ||
		 $this->type == self::IMAGESHOW_TYPE) {
			$this->filename = $content['filename'];
		}
	}
	/**
	 * @return int the item $type
	 */
	function getType ()
	{
		return $this->type;
	}
	function __toStringZip ()
	{
		if ($this->type != self::IMAGESHOW_TYPE) {
			return false;
		}
		$zip = new ZipArchive();
		if ($zip->open($this->filename) === true) {
			$_list = array();
			for ($i = 0; $i < $zip->numFiles; $i ++) {
				$item = $zip->statIndex($i);
				if (isset($item['name']) && in_array(
				pathinfo($item['name'], PATHINFO_EXTENSION),
				array(
					'jpeg',
					'jpg',
					'png'))) {
					$_list[$i] = $item['name'];
				}
			}
			natsort($_list);
			$_return = null;
			foreach ($_list as $i => $filename) {
				$_return .= pack('cccV', $this->type, $this->duration, $i,
				$this->media_id);
				$_return .= pack('Va5', 11 + 5,
				pathinfo($filename, PATHINFO_EXTENSION));
			}
			$zip->close();
			return $_return;
		}
		return false;
	}
	/**
	 * Produces a binary string containing all necessary item data
	 * @return string
	 */
	public function __toString ()
	{
		if ($this->type == self::IMAGESHOW_TYPE) {
			return $this->__toStringZip();
		}
		// TODO: add a byte (or bit) somewhere to force redownload
		$binary = pack('cccV', $this->type, $this->duration, 0,
		$this->media_id);
		if ($this->type == self::IMAGE_TYPE || $this->type == self::VIDEO_TYPE ||
		 $this->type == self::POWERPOINT_TYPE) {
			// 11 for the item/type headers and 5 for the extension
			$binary .= pack('Va5', 11 + 5,
			pathinfo($this->filename, PATHINFO_EXTENSION));
		}
		return $binary;
	}
}