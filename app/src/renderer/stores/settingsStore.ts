import { defineStore } from 'pinia'
import { watch, ref } from 'vue'
import type { LlmConfig } from '../../shared/types/sensenode'

export type Locale = 'zh-CN' | 'en-US'
export type ThemeMode = 'dark' | 'light'

const storageKeys = {
	locale: 'sensenode.locale',
	themeMode: 'sensenode.themeMode.v2',
} as const

function readStoredValue<T extends string>(key: string, fallback: T): T {
	if (typeof window === 'undefined') {
		return fallback
	}

	const value = window.localStorage.getItem(key)
	return (value as T | null) ?? fallback
}

export const useSettingsStore = defineStore('settings', () => {
	const miniHudEnabled = ref(true)
	const autoReconnect = ref(true)
	const eventStabilizerEnabled = ref(true)
	const locale = ref<Locale>(readStoredValue(storageKeys.locale, 'zh-CN'))
	const themeMode = ref<ThemeMode>(
		readStoredValue(storageKeys.themeMode, 'light'),
	)
	const themeAccent = ref('#9BA6B2')
	const llmConfig = ref<LlmConfig>({
		provider: 'none',
		endpoint: 'http://localhost:11434',
		model: '',
	})

	watch(locale, (value) => {
		if (typeof window !== 'undefined') {
			window.localStorage.setItem(storageKeys.locale, value)
		}
	})

	watch(themeMode, (value) => {
		if (typeof window !== 'undefined') {
			window.localStorage.setItem(storageKeys.themeMode, value)
		}
	})

	return {
		miniHudEnabled,
		autoReconnect,
		eventStabilizerEnabled,
		locale,
		themeMode,
		themeAccent,
		llmConfig,
	}
})
