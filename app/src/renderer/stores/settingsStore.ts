import { defineStore } from 'pinia'
import { computed, watch, ref } from 'vue'
import type { Locale } from '../../shared/i18n'

export type EffectiveThemeMode = 'dark' | 'light'
export type ThemeMode = EffectiveThemeMode | 'system'

const storageKeys = {
	locale: 'sensenode.locale',
	themeMode: 'sensenode.themeMode.v4',
} as const

function readStoredValue<T extends string>(key: string, fallback: T): T {
	if (typeof window === 'undefined') {
		return fallback
	}

	const value = window.localStorage.getItem(key)
	return (value as T | null) ?? fallback
}

function readStoredThemeMode(): ThemeMode {
	const value = readStoredValue(storageKeys.themeMode, 'system')
	if (value === 'dark' || value === 'light' || value === 'system') {
		return value
	}

	return 'system'
}

function readSystemThemeMode(): EffectiveThemeMode {
	if (typeof window === 'undefined') {
		return 'light'
	}

	return window.matchMedia('(prefers-color-scheme: dark)').matches
		? 'dark'
		: 'light'
}

export const useSettingsStore = defineStore('settings', () => {
	const autoReconnect = ref(true)
	const eventStabilizerEnabled = ref(true)
	const locale = ref<Locale>(readStoredValue(storageKeys.locale, 'zh-CN'))
	const themeMode = ref<ThemeMode>(readStoredThemeMode())
	const systemThemeMode = ref<EffectiveThemeMode>(readSystemThemeMode())
	const effectiveThemeMode = computed<EffectiveThemeMode>(() =>
		themeMode.value === 'system' ? systemThemeMode.value : themeMode.value,
	)
	const themeAccent = ref('#9BA6B2')

	if (typeof window !== 'undefined') {
		const mediaQuery = window.matchMedia('(prefers-color-scheme: dark)')
		const updateSystemThemeMode = (event: MediaQueryListEvent) => {
			systemThemeMode.value = event.matches ? 'dark' : 'light'
		}

		mediaQuery.addEventListener('change', updateSystemThemeMode)
	}

	watch(
		locale,
		(value) => {
			if (typeof window !== 'undefined') {
				window.localStorage.setItem(storageKeys.locale, value)
				window.sensenode?.app?.setLocale?.(value)
			}
		},
		{ immediate: true },
	)

	watch(themeMode, (value) => {
		if (typeof window !== 'undefined') {
			window.localStorage.setItem(storageKeys.themeMode, value)
		}
	})

	return {
		autoReconnect,
		eventStabilizerEnabled,
		locale,
		themeMode,
		systemThemeMode,
		effectiveThemeMode,
		themeAccent,
	}
})
