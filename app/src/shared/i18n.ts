export type Locale = 'zh-CN' | 'en-US'

export function normalizeLocale(input: string | null | undefined): Locale {
	if (input?.toLowerCase().startsWith('zh')) {
		return 'zh-CN'
	}

	return 'en-US'
}
