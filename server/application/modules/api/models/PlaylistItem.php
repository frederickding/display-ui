<?php
/**
 * Playlist Item
 *
 * Copyright 2009 Frederick Ding<br />
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 *
 * You may obtain a copy of the License at
 * 		http://www.apache.org/licenses/LICENSE-2.0
 * or the full licensing terms for this project at
 * 		http://code.google.com/p/display-ui/wiki/License
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
	const IMAGE_TYPE = 1;
	const TEXT_TYPE = 2;
	const VIDEO_TYPE = 3;
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
	const TRANS_FADE = 0;
	/**
	 * The transition between images
	 * @var int
	 */
	private $transition = 0;
	private $width = 0;
	private $height = 0;
	private $filename = '';
	/**
	 * Builds a playlist item with the specified type and content
	 * @param int $type
	 * @param array $content
	 * @param int $duration
	 * @param int $trans
	 */
	public function __construct($type, $content, $duration = 20, $trans = 0)
	{
		if(1 <= $type && $type <= 3) {
			$this->type = $type;
		} else die('Bad type for Api_Playlist_Item.');
		$this->duration = $duration;
		$this->transition = $trans;
		if($this->type == self::IMAGE_TYPE || $this->type == self::VIDEO_TYPE) {
			$this->width = $content['width'];
			$this->height = $content['height'];
			$this->filename = $content['filename'];
		} elseif($this->type == self::TEXT_TYPE) {
			$this->filename = $content['filename'];
		}
	}
	/**
	 * Produces a binary string containing all necessary item data
	 * @return string
	 */
	public function __toString()
	{
		$binary = pack('ccv', $this->type, $this->duration, $this->transition);
		if($this->type == self::IMAGE_TYPE) {
			$binary .= pack('VvvV', 16 + 2*strlen($this->filename),
						$this->width, $this->height, strlen($this->filename));
			$binary .= iconv('UTF-8', 'UTF-16LE', $this->filename);
		}
		return $binary;
	}

}