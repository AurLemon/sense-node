<script setup lang="ts">
import { computed, onBeforeUnmount, onMounted, ref, watch } from 'vue'
import * as echarts from 'echarts'
import type { ECharts, EChartsOption, LineSeriesOption } from 'echarts'
import { useI18n } from '../../lib/i18n'
import { useDeviceStore } from '../../stores/deviceStore'
import { useSerialStore } from '../../stores/serialStore'
import { useSettingsStore } from '../../stores/settingsStore'
import type { SenseNodeFrame } from '../../../shared/types/sensenode'

interface DisplayFrame extends SenseNodeFrame {
	displayTimeMs: number
}

interface TelemetryChartState {
	frameCount: number
	domainStartMs: number
	domainEndMs: number
	imuSeries: Array<[number, number | null]>
	tofSeries: Array<[number, number | null]>
}

const chartWindowMs = 60 * 1000
const resetThresholdMs = 5_000
const xTickIntervalMs = 60 * 1000

const device = useDeviceStore()
const serial = useSerialStore()
const settings = useSettingsStore()
const { t } = useI18n()

const imuChartEl = ref<HTMLDivElement | null>(null)
const tofChartEl = ref<HTMLDivElement | null>(null)

let imuChart: ECharts | null = null
let tofChart: ECharts | null = null
let resizeObserver: ResizeObserver | null = null

const isConnected = computed(() => serial.status.state === 'connected')

const chartState = computed<TelemetryChartState>(() => {
	const emptyState: TelemetryChartState = {
		frameCount: 0,
		domainStartMs: 0,
		domainEndMs: chartWindowMs,
		imuSeries: [],
		tofSeries: [],
	}

	if (!isConnected.value) {
		return emptyState
	}

	const sensorFrames = device.frames
		.filter(
			(frame) => frame.type === 'sensor_frame' && frame.uptime_ms !== undefined,
		)
		.toReversed()

	if (sensorFrames.length === 0) {
		return emptyState
	}

	const displayFrames = buildDisplayFrames(sensorFrames)
	const latestDisplayTimeMs = displayFrames.at(-1)?.displayTimeMs ?? 0
	const domainEndMs =
		latestDisplayTimeMs < chartWindowMs ? chartWindowMs : latestDisplayTimeMs
	const domainStartMs =
		latestDisplayTimeMs < chartWindowMs ? 0 : domainEndMs - chartWindowMs

	const visibleFrames = displayFrames.filter(
		(frame) =>
			frame.displayTimeMs >= domainStartMs &&
			frame.displayTimeMs <= domainEndMs,
	)

	return {
		frameCount: visibleFrames.length,
		domainStartMs,
		domainEndMs,
		imuSeries: buildImuSeries(visibleFrames),
		tofSeries: visibleFrames
			.map(
				(frame) =>
					[frame.displayTimeMs, normalizeTofValue(frame.tof_mm)] as [
						number,
						number | null,
					],
			)
			.filter((point) => point[1] !== null),
	}
})

const hasChartData = computed(() => chartState.value.frameCount > 0)

function normalizeTofValue(value?: number): number | null {
	if (value === undefined || value < 0 || value === 8191) {
		return null
	}

	return value
}

function buildDisplayFrames(frames: SenseNodeFrame[]): DisplayFrame[] {
	const output: DisplayFrame[] = []
	let offsetMs = 0
	let previousUptimeMs: number | null = null
	let previousDisplayTimeMs = 0

	for (const frame of frames) {
		const uptimeMs = frame.uptime_ms
		if (uptimeMs === undefined) {
			continue
		}

		if (
			previousUptimeMs !== null &&
			uptimeMs + resetThresholdMs < previousUptimeMs
		) {
			offsetMs = previousDisplayTimeMs
		}

		const displayTimeMs = uptimeMs + offsetMs
		output.push({
			...frame,
			displayTimeMs,
		})

		previousUptimeMs = uptimeMs
		previousDisplayTimeMs = displayTimeMs
	}

	return output
}

function buildImuSeries(
	frames: DisplayFrame[],
): Array<[number, number | null][]> {
	return [
		frames.map((frame) => [frame.displayTimeMs, frame.accel?.x ?? null]),
		frames.map((frame) => [frame.displayTimeMs, frame.accel?.y ?? null]),
		frames.map((frame) => [frame.displayTimeMs, frame.accel?.z ?? null]),
		frames.map((frame) => [frame.displayTimeMs, frame.gyro?.x ?? null]),
		frames.map((frame) => [frame.displayTimeMs, frame.gyro?.y ?? null]),
		frames.map((frame) => [frame.displayTimeMs, frame.gyro?.z ?? null]),
	]
}

function formatDuration(ms: number): string {
	const totalSeconds = Math.max(0, Math.floor(ms / 1000))
	const minutes = Math.floor(totalSeconds / 60)
	const seconds = totalSeconds % 60
	return `${String(minutes).padStart(2, '0')}:${String(seconds).padStart(2, '0')}`
}

function readChartTheme() {
	const style = getComputedStyle(document.documentElement)
	return {
		fontFamily: style.fontFamily || "'Rubik', 'MiSans', system-ui, sans-serif",
		textColor: style.getPropertyValue('--color-text--muted').trim(),
		textStrongColor: style.getPropertyValue('--color-text').trim(),
		gridColor: style.getPropertyValue('--border-color-base').trim(),
		splitLineColor: style
			.getPropertyValue('--border-color-base--lighter')
			.trim(),
		backgroundColor: 'transparent',
		axisLineColor: style.getPropertyValue('--border-color-base--darker').trim(),
		tooltipBackground: style
			.getPropertyValue('--background-color-overlay')
			.trim(),
		tooltipBorderColor: style.getPropertyValue('--border-color-base').trim(),
		primary500: style.getPropertyValue('--ui-color-primary-500').trim(),
		success500: style.getPropertyValue('--ui-color-success-500').trim(),
		warning500: style.getPropertyValue('--ui-color-warning-500').trim(),
		neutral600: style.getPropertyValue('--ui-color-neutral-600').trim(),
		teal500: '#14b8a6',
		orange500: '#f97316',
	}
}

function buildBaseOption(
	yLabel: string,
	domainStartMs: number,
	domainEndMs: number,
): EChartsOption {
	const theme = readChartTheme()

	return {
		backgroundColor: theme.backgroundColor,
		animation: false,
		textStyle: {
			fontFamily: theme.fontFamily,
			color: theme.textColor,
		},
		grid: {
			left: 52,
			right: 12,
			top: 24,
			bottom: 30,
			containLabel: false,
		},
		tooltip: {
			show: false,
			trigger: 'none',
		},
		axisPointer: {
			show: false,
		},
		xAxis: {
			type: 'value',
			min: domainStartMs,
			max: domainEndMs,
			interval: xTickIntervalMs,
			boundaryGap: [0, 0],
			axisLabel: {
				color: theme.textColor,
				formatter(value: number) {
					return formatDuration(value)
				},
			},
			axisLine: {
				lineStyle: {
					color: theme.axisLineColor,
				},
			},
			axisTick: {
				show: false,
			},
			splitLine: {
				show: true,
				lineStyle: {
					color: theme.splitLineColor,
				},
			},
		},
		yAxis: {
			type: 'value',
			name: yLabel,
			nameLocation: 'middle',
			nameGap: 42,
			scale: true,
			axisLabel: {
				color: theme.textColor,
			},
			nameTextStyle: {
				color: theme.textColor,
				fontFamily: theme.fontFamily,
				fontWeight: 600,
			},
			axisLine: {
				show: false,
			},
			axisTick: {
				show: false,
			},
			splitLine: {
				show: true,
				lineStyle: {
					color: theme.splitLineColor,
				},
			},
		},
	}
}

function buildImuOption(): EChartsOption {
	const state = chartState.value
	const theme = readChartTheme()
	const baseOption = buildBaseOption(
		'IMU',
		state.domainStartMs,
		state.domainEndMs,
	)
	const seriesDefs: Array<{
		name: string
		color: string
		data: Array<[number, number | null]>
	}> = [
		{ name: 'AX', color: theme.primary500, data: state.imuSeries[0] ?? [] },
		{ name: 'AY', color: theme.success500, data: state.imuSeries[1] ?? [] },
		{ name: 'AZ', color: theme.warning500, data: state.imuSeries[2] ?? [] },
		{ name: 'GX', color: theme.neutral600, data: state.imuSeries[3] ?? [] },
		{ name: 'GY', color: theme.teal500, data: state.imuSeries[4] ?? [] },
		{ name: 'GZ', color: theme.orange500, data: state.imuSeries[5] ?? [] },
	]

	return {
		...baseOption,
		color: seriesDefs.map((item) => item.color),
		legend: {
			top: 0,
			right: 0,
			icon: 'circle',
			itemWidth: 10,
			itemHeight: 10,
			textStyle: {
				color: theme.textColor,
				fontFamily: theme.fontFamily,
			},
		},
		series: seriesDefs.map(
			(item): LineSeriesOption => ({
				name: item.name,
				type: 'line',
				showSymbol: false,
				connectNulls: false,
				smooth: false,
				sampling: 'lttb',
				data: item.data,
				lineStyle: {
					width: 1.75,
					color: item.color,
				},
			}),
		),
	}
}

function buildTofOption(): EChartsOption {
	const state = chartState.value
	const theme = readChartTheme()
	const baseOption = buildBaseOption(
		'mm',
		state.domainStartMs,
		state.domainEndMs,
	)

	return {
		...baseOption,
		color: [theme.primary500],
		legend: {
			top: 0,
			right: 0,
			icon: 'circle',
			itemWidth: 10,
			itemHeight: 10,
			textStyle: {
				color: theme.textColor,
				fontFamily: theme.fontFamily,
			},
			data: ['ToF'],
		},
		series: [
			{
				name: 'ToF',
				type: 'line',
				showSymbol: false,
				connectNulls: false,
				smooth: false,
				sampling: 'lttb',
				data: state.tofSeries,
				lineStyle: {
					width: 2,
					color: theme.primary500,
				},
			},
		],
	}
}

function renderCharts(): void {
	if (!imuChartEl.value || !tofChartEl.value) {
		return
	}

	if (!imuChart) {
		imuChart = echarts.init(imuChartEl.value)
	}

	if (!tofChart) {
		tofChart = echarts.init(tofChartEl.value)
	}

	imuChart.setOption(buildImuOption(), true)
	tofChart.setOption(buildTofOption(), true)
}

function resizeCharts(): void {
	imuChart?.resize()
	tofChart?.resize()
}

onMounted(() => {
	renderCharts()

	resizeObserver = new ResizeObserver(() => {
		resizeCharts()
	})

	if (imuChartEl.value) {
		resizeObserver.observe(imuChartEl.value)
	}

	if (tofChartEl.value) {
		resizeObserver.observe(tofChartEl.value)
	}
})

watch(
	() => [chartState.value, settings.themeMode, settings.locale] as const,
	() => {
		renderCharts()
	},
	{ deep: true },
)

onBeforeUnmount(() => {
	resizeObserver?.disconnect()
	resizeObserver = null
	imuChart?.dispose()
	tofChart?.dispose()
	imuChart = null
	tofChart = null
})
</script>

<template>
	<div
		class="grid gap-3 rounded-lg border border-default bg-default/75 p-3 backdrop-blur"
	>
		<div class="flex items-center justify-between gap-3">
			<div>
				<div class="text-sm font-medium text-highlighted">
					{{ t('chart.telemetry') }}
				</div>
				<div class="mt-0.5 text-xs text-muted">
					{{ t('chart.window5m') }}
				</div>
			</div>
			<UBadge color="neutral" variant="soft">
				{{ chartState.frameCount }}
			</UBadge>
		</div>

		<div v-if="hasChartData" class="grid gap-3">
			<div class="grid gap-1.5">
				<div class="text-xs text-muted">{{ t('chart.imu') }}</div>
				<div ref="imuChartEl" class="h-[180px] w-full"></div>
			</div>
			<div class="grid gap-1.5">
				<div class="text-xs text-muted">{{ t('chart.tof') }}</div>
				<div ref="tofChartEl" class="h-[140px] w-full"></div>
			</div>
		</div>
		<div v-else class="grid h-52 place-items-center text-sm text-muted">
			{{ t('chart.waiting') }}
		</div>
	</div>
</template>
