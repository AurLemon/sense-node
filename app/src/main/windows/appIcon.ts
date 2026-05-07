import { app, nativeImage, nativeTheme } from 'electron'
import path from 'node:path'

const iconBaseName = 'app-icon'
const darkIconBaseName = 'app-icon-dark'

function resolveAssetDir(): string {
	if (app.isPackaged) {
		return path.join(process.resourcesPath, 'app.asar', 'src/main/assets')
	}

	return path.join(app.getAppPath(), 'src/main/assets')
}

function resolveWindowIconPath(): string {
	const assetDir = resolveAssetDir()
	const themedIconBaseName = resolveThemeIconBaseName()
	if (process.platform === 'win32') {
		return path.join(assetDir, `${themedIconBaseName}.ico`)
	}

	return path.join(assetDir, `${themedIconBaseName}.png`)
}

function resolveTrayIconPath(): string {
	const assetDir = resolveAssetDir()
	const themedIconBaseName = resolveThemeIconBaseName()
	if (process.platform === 'win32') {
		return path.join(assetDir, `${themedIconBaseName}-32.png`)
	}

	return path.join(assetDir, `${themedIconBaseName}.png`)
}

function resolveThemeIconBaseName(): string {
	return nativeTheme.shouldUseDarkColors ? darkIconBaseName : iconBaseName
}

export function getAppIcon(): Electron.NativeImage {
	return nativeImage.createFromPath(resolveWindowIconPath())
}

export function getTrayIcon(): Electron.NativeImage {
	return nativeImage.createFromPath(resolveTrayIconPath())
}
