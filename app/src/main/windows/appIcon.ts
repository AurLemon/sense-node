import { app, nativeImage } from 'electron'
import path from 'node:path'

const assetDir = path.join(app.getAppPath(), 'src/main/assets')
const iconBaseName = 'app-icon'

function resolveIconPath(): string {
	if (process.platform === 'win32') {
		return path.join(assetDir, `${iconBaseName}.ico`)
	}

	return path.join(assetDir, `${iconBaseName}.png`)
}

export function getAppIcon(): Electron.NativeImage {
	return nativeImage.createFromPath(resolveIconPath())
}
