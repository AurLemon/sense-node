import { app, nativeImage } from 'electron'
import path from 'node:path'

let cachedIcon: Electron.NativeImage | null = null

export function getAppIcon(): Electron.NativeImage {
	if (cachedIcon) {
		return cachedIcon
	}

	const pngPath = path.join(app.getAppPath(), 'src/main/assets/app-icon.png')
	cachedIcon = nativeImage.createFromPath(pngPath)
	return cachedIcon
}
